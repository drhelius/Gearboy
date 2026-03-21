/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef MCP_TRANSPORT_H
#define MCP_TRANSPORT_H

#define MCP_HTTP_ENDPOINT_PATH "/mcp"

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include "log.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_CLOSE(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_CLOSE(s) ::close(s)
#endif

class McpTransportInterface
{
public:
    virtual ~McpTransportInterface() {}
    virtual bool send(const std::string& jsonLine) = 0;
    virtual bool recv(std::string& jsonLine) = 0;
    virtual void close() = 0;
};

class StdioTransport : public McpTransportInterface
{
public:
    StdioTransport()
    {
        m_closed = false;
    }
    ~StdioTransport() {}

    bool send(const std::string& jsonLine)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed)
            return false;
        std::cout << jsonLine << "\n";
        std::cout.flush();
        return true;
    }

    bool recv(std::string& jsonLine)
    {
        if (m_closed)
            return false;
        return static_cast<bool>(std::getline(std::cin, jsonLine));
    }

    void close()
    {
        m_closed = true;
    }

private:
    std::mutex m_mutex;
    bool m_closed;
};

// HttpTransport - Simple HTTP server for MCP over HTTP POST
class HttpTransport : public McpTransportInterface
{
public:
    HttpTransport(int port)
    {
        m_closed = false;
        m_server_socket = INVALID_SOCKET_VALUE;
        m_current_client = INVALID_SOCKET_VALUE;
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_server_socket == INVALID_SOCKET_VALUE)
        {
            Error("[MCP] Failed to create socket");
            return;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(m_server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            Error("[MCP] Failed to bind to port %d", port);
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
            return;
        }

        if (listen(m_server_socket, 5) < 0)
        {
            Error("[MCP] Failed to listen on socket");
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
            return;
        }

        Log("[MCP] HTTP server listening on http://127.0.0.1:%d", port);
    }

    ~HttpTransport()
    {
        close();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool send(const std::string& jsonLine)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed || m_current_client == INVALID_SOCKET_VALUE)
        {
            Debug("[MCP] HTTP send failed: %s (closed=%d, client=%d)", 
                m_closed ? "server closed" : "no client connected",
                m_closed, (int)m_current_client);
            return false;
        }

        // Build HTTP response (close connection after each response for simplicity)
        std::string http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(jsonLine.length()) + "\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "\r\n" +
            jsonLine;

        int sent = ::send(m_current_client, http_response.c_str(), (int)http_response.length(), 0);

        if (sent <= 0)
        {
            Error("[MCP] HTTP send() failed: %d", sent);
            SOCKET_CLOSE(m_current_client);
            m_current_client = INVALID_SOCKET_VALUE;
            m_client_cv.notify_all();
            return false;
        }

        std::string log_output = jsonLine.length() > 500 
            ? jsonLine.substr(0, 500) + "..." 
            : jsonLine;
        Debug("[MCP] HTTP response sent (%d bytes): %s", 
            sent, log_output.c_str());

        // Close connection after response (VS Code will reconnect for next request)
        SOCKET_CLOSE(m_current_client);
        m_current_client = INVALID_SOCKET_VALUE;
        m_client_cv.notify_all();

        return true;
    }

    // Helper function to extract path from HTTP request line
    std::string extract_http_path(const std::string& request)
    {
        // Request format: "METHOD /path HTTP/1.1\r\n..."
        size_t method_end = request.find(' ');
        if (method_end == std::string::npos)
            return "";

        size_t path_start = method_end + 1;
        size_t path_end = request.find(' ', path_start);
        if (path_end == std::string::npos)
            return "";

        return request.substr(path_start, path_end - path_start);
    }

    // Helper function to validate path and send 404 if invalid
    // Returns true if path is valid, false if rejected (and response already sent)
    bool validate_and_reject_invalid_path(const std::string& request, socket_t client)
    {
        std::string path = extract_http_path(request);
        if (path == MCP_HTTP_ENDPOINT_PATH)
            return true;

        Debug("[MCP] Rejecting request to invalid path: %s", path.c_str());
        const char* not_found_response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found";
        ::send(client, not_found_response, (int)strlen(not_found_response), 0);
        SOCKET_CLOSE(client);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_current_client = INVALID_SOCKET_VALUE;
            m_client_cv.notify_all();
        }
        return false;
    }

    bool recv(std::string& jsonLine)
    {
        while (!m_closed && m_server_socket != INVALID_SOCKET_VALUE)
        {
            // Wait until any previous client has been responded to and closed
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_client_cv.wait(lock, [this]
                {
                    return m_closed || m_current_client == INVALID_SOCKET_VALUE;
                });

                if (m_closed)
                {
                    Debug("[MCP] Server closed, stopping recv loop");
                    return false;
                }
            }

            // Accept new connection (we close after each response)
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            socket_t client = accept(m_server_socket, (struct sockaddr*)&client_addr, &client_len);

            if (client == INVALID_SOCKET_VALUE)
            {
                if (m_closed)
                {
                    Debug("[MCP] Server closed, stopping recv loop");
                    return false;
                }
                Debug("[MCP] accept() failed, retrying");
                continue; // Try again
            }

            Log("[MCP] HTTP client connected");

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_current_client = client;
            }

            // Read HTTP request (can be larger than one recv call)
            std::string request;
            char buffer[4096];
            int header_end = -1;
            int content_length = -1;
            bool read_error = false;
            bool is_options = false;
            bool connection_closed = false;

            while (true)
            {
                int received = ::recv(client, buffer, sizeof(buffer), 0);
                if (received <= 0)
                {
                    read_error = true;
                    connection_closed = (received == 0);
                    Debug("[MCP] HTTP recv %s (received=%d)", 
                            connection_closed ? "connection closed by client" : "error",
                            received);
                    break;
                }

                request.append(buffer, received);

                // Look for end of headers
                if (header_end == -1)
                {
                    size_t pos = request.find("\r\n\r\n");
                    if (pos != std::string::npos)
                    {
                        header_end = (int)(pos + 4);

                        // Log the full request headers for debugging
                        Debug("[MCP] HTTP headers:\n%s", request.substr(0, header_end).c_str());

                        // Check if this is an OPTIONS request (CORS preflight)
                        if (request.find("OPTIONS ") == 0)
                        {
                            if (!validate_and_reject_invalid_path(request, client))
                                continue;

                            is_options = true;
                            content_length = 0;
                            break;
                        }

                        // Parse Content-Length (case-insensitive)
                        size_t cl_pos = request.find("Content-Length:");
                        if (cl_pos == std::string::npos)
                            cl_pos = request.find("content-length:");

                        if (cl_pos != std::string::npos)
                        {
                            const char* cl_start = request.c_str() + cl_pos;
                            while (*cl_start && *cl_start != ':') cl_start++;
                            if (*cl_start == ':') cl_start++;
                            while (*cl_start && (*cl_start == ' ' || *cl_start == '\t')) cl_start++;
                            content_length = atoi(cl_start);
                        }
                        else
                        {
                            // No Content-Length header, assume no body
                            content_length = 0;
                        }
                    }
                }

                // Check if we have the complete body
                if (header_end > 0 && content_length >= 0)
                {
                    int body_length = (int)request.length() - header_end;
                    if (body_length >= content_length)
                        break;
                }
            }

            // Handle connection closed by client
            if (connection_closed)
            {
                Log("[MCP] Client disconnected, cleaning up");
                std::lock_guard<std::mutex> lock(m_mutex);
                SOCKET_CLOSE(m_current_client);
                m_current_client = INVALID_SOCKET_VALUE;
                m_client_cv.notify_all();
                continue; // Wait for next connection
            }

            // Handle OPTIONS request (CORS preflight)
            if (is_options)
            {
                Debug("[MCP] HTTP OPTIONS request (CORS preflight)");

                std::string options_response = 
                    "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, Accept, MCP-Session-Id, MCP-Protocol-Version\r\n"
                    "Connection: close\r\n"
                    "\r\n";

                ::send(client, options_response.c_str(), (int)options_response.length(), 0);

                std::lock_guard<std::mutex> lock(m_mutex);
                SOCKET_CLOSE(m_current_client);
                m_current_client = INVALID_SOCKET_VALUE;
                m_client_cv.notify_all();
                continue; // Wait for next connection
            }

            // Handle POST with body (normal MCP request)
            if (!read_error && header_end > 0 && content_length > 0)
            {
                if (!validate_and_reject_invalid_path(request, client))
                    continue;

                jsonLine = request.substr(header_end, content_length);
                Debug("[MCP] HTTP POST request received (%d bytes): %s", 
                    (int)jsonLine.length(), 
                    jsonLine.substr(0, 100).c_str());
                return true;
            }

            // Handle GET request - client wants to open SSE stream for async notifications
            // We don't support SSE, so respond with 405 Method Not Allowed
            if (!read_error && header_end > 0 && request.find("GET ") == 0)
            {
                if (!validate_and_reject_invalid_path(request, client))
                    continue;
                
                Debug("[MCP] HTTP GET request (SSE not supported, responding 405)");

                std::string method_not_allowed = 
                    "HTTP/1.1 405 Method Not Allowed\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 56\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "{\"error\":\"SSE streaming not supported by this server\"}";

                ::send(client, method_not_allowed.c_str(), (int)method_not_allowed.length(), 0);

                std::lock_guard<std::mutex> lock(m_mutex);
                SOCKET_CLOSE(m_current_client);
                m_current_client = INVALID_SOCKET_VALUE;
                m_client_cv.notify_all();
                continue; // Wait for next request
            }

            // Handle POST without body - VS Code MCP client doesn't do health checks this way
            // According to spec, all MCP messages should have a JSON-RPC body
            if (!read_error && header_end > 0 && content_length == 0 && request.find("POST ") == 0)
            {
                if (!validate_and_reject_invalid_path(request, client))
                    continue;

                Debug("[MCP] HTTP POST without body (non-standard, closing connection)");

                std::lock_guard<std::mutex> lock(m_mutex);
                SOCKET_CLOSE(m_current_client);
                m_current_client = INVALID_SOCKET_VALUE;
                m_client_cv.notify_all();
                continue; // Wait for next request
            }

            // Unknown/malformed request - log and close connection
            if (read_error && !connection_closed)
            {
                Debug("[MCP] HTTP read error (header_end=%d, content_length=%d)", 
                        header_end, content_length);

                size_t first_line_end = request.find("\r\n");
                if (first_line_end != std::string::npos)
                {
                    Debug("[MCP] Request line: %s", request.substr(0, first_line_end).c_str());
                }

                std::lock_guard<std::mutex> lock(m_mutex);
                SOCKET_CLOSE(m_current_client);
                m_current_client = INVALID_SOCKET_VALUE;
                m_client_cv.notify_all();
                continue; // Wait for next connection
            }
        }

        return false; // Server closed
    }

    void close()
    {
        m_closed = true;
        m_client_cv.notify_all();

        if (m_current_client != INVALID_SOCKET_VALUE)
        {
            SOCKET_CLOSE(m_current_client);
            m_current_client = INVALID_SOCKET_VALUE;
        }

        if (m_server_socket != INVALID_SOCKET_VALUE)
        {
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_client_cv;
    bool m_closed;
    socket_t m_server_socket;
    socket_t m_current_client;
};

#endif /* MCP_TRANSPORT_H */
