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
#define MCP_HTTP_AUTH_ENV "GEARBOY_MCP_HTTP_TOKEN"

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "log.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    typedef int socket_len_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_CLOSE(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int socket_t;
    typedef socklen_t socket_len_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_CLOSE(s) ::close(s)
#endif

class McpTransportInterface
{
public:
    virtual ~McpTransportInterface() {}
    virtual bool send(const std::string& jsonLine) = 0;
    virtual bool acknowledge_notification() = 0;
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

    bool acknowledge_notification()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_closed;
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
    HttpTransport(const std::string& address, int port)
    {
        m_closed = false;
        m_port = port;
        m_bind_address = address.empty() ? "127.0.0.1" : address;
        if (m_bind_address == "localhost")
            m_bind_address = "127.0.0.1";
        const char* auth_token = getenv(MCP_HTTP_AUTH_ENV);
        if (auth_token && auth_token[0])
            m_auth_token = auth_token;
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
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, m_bind_address.c_str(), &addr.sin_addr) != 1)
        {
            Error("[MCP] Invalid HTTP bind address: %s", m_bind_address.c_str());
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
            return;
        }

        if (!is_loopback_bind_address() && m_auth_token.empty())
        {
            Error("[MCP] HTTP bearer token %s is required for non-loopback bind address %s", MCP_HTTP_AUTH_ENV, m_bind_address.c_str());
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
            return;
        }

        if (bind(m_server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            Error("[MCP] Failed to bind to %s:%d", m_bind_address.c_str(), port);
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

        Log("[MCP] HTTP server listening on http://%s:%d%s", m_bind_address.c_str(), port, MCP_HTTP_ENDPOINT_PATH);
        if (!m_auth_token.empty())
            Log("[MCP] HTTP bearer token authentication enabled from %s", MCP_HTTP_AUTH_ENV);
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
        return send_http_response(200, "OK", "application/json", jsonLine, true);
    }

    bool acknowledge_notification()
    {
        return send_http_response(202, "Accepted", NULL, "", true);
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

    static char ascii_lower(char value)
    {
        if ((value >= 'A') && (value <= 'Z'))
            return value + ('a' - 'A');

        return value;
    }

    bool header_name_matches(const std::string& request, size_t line_start, size_t line_end, const char* header_name)
    {
        size_t name_length = strlen(header_name);
        if ((line_end - line_start) <= name_length)
            return false;

        if (request[line_start + name_length] != ':')
            return false;

        for (size_t i = 0; i < name_length; i++)
        {
            if (ascii_lower(request[line_start + i]) != ascii_lower(header_name[i]))
                return false;
        }

        return true;
    }

    int extract_http_header(const std::string& request, const char* header_name, std::string& value)
    {
        value.clear();
        int count = 0;
        size_t line_start = request.find("\r\n");
        if (line_start == std::string::npos)
            return 0;

        line_start += 2;

        while (line_start < request.length())
        {
            size_t line_end = request.find("\r\n", line_start);
            if (line_end == std::string::npos)
                return count;

            if (line_end == line_start)
                return count;

            if (header_name_matches(request, line_start, line_end, header_name))
            {
                count++;
                size_t value_start = line_start + strlen(header_name) + 1;
                while ((value_start < line_end) && ((request[value_start] == ' ') || (request[value_start] == '\t')))
                    value_start++;

                size_t value_end = line_end;
                while ((value_end > value_start) && ((request[value_end - 1] == ' ') || (request[value_end - 1] == '\t')))
                    value_end--;

                if (count == 1)
                    value = request.substr(value_start, value_end - value_start);
            }

            line_start = line_end + 2;
        }

        return count;
    }

    std::string sanitize_http_headers_for_debug(const std::string& headers)
    {
        std::string sanitized;
        size_t line_start = 0;

        while (line_start < headers.length())
        {
            size_t line_end = headers.find("\r\n", line_start);
            if (line_end == std::string::npos)
            {
                sanitized += headers.substr(line_start);
                break;
            }

            if (header_name_matches(headers, line_start, line_end, "Authorization"))
                sanitized += "Authorization: <redacted>\r\n";
            else
                sanitized += headers.substr(line_start, line_end - line_start + 2);

            line_start = line_end + 2;
        }

        return sanitized;
    }

    bool host_matches_address(const std::string& host, const std::string& address)
    {
        std::string port = std::to_string(m_port);
        return (host == address) || (host == address + ":" + port);
    }

    bool origin_matches_address(const std::string& origin, const std::string& address)
    {
        std::string port = std::to_string(m_port);
        return origin == "http://" + address + ":" + port;
    }

    bool is_loopback_address(const std::string& address)
    {
        struct in_addr parsed_address;
        if (inet_pton(AF_INET, address.c_str(), &parsed_address) != 1)
            return false;

        return (ntohl(parsed_address.s_addr) & 0xFF000000) == 0x7F000000;
    }

    bool is_loopback_bind_address()
    {
        return is_loopback_address(m_bind_address);
    }

    bool get_socket_local_address(socket_t client, std::string& address)
    {
        struct sockaddr_in local_address;
        memset(&local_address, 0, sizeof(local_address));
        socket_len_t local_length = sizeof(local_address);
        if (getsockname(client, (struct sockaddr*)&local_address, &local_length) != 0 || local_address.sin_family != AF_INET)
            return false;

        char buffer[INET_ADDRSTRLEN];
        if (!inet_ntop(AF_INET, &local_address.sin_addr, buffer, sizeof(buffer)))
            return false;

        address = buffer;
        return true;
    }

    bool is_allowed_host(const std::string& host, const std::string& local_address)
    {
        if (host_matches_address(host, local_address))
            return true;

        return is_loopback_address(local_address) && host_matches_address(host, "localhost");
    }

    bool is_allowed_origin(const std::string& origin, const std::string& local_address)
    {
        if (origin_matches_address(origin, local_address))
            return true;

        return is_loopback_address(local_address) && origin_matches_address(origin, "localhost");
    }

    std::string get_cors_origin_header()
    {
        if (m_current_origin.empty())
            return "";

        return "Access-Control-Allow-Origin: " + m_current_origin + "\r\n"
               "Vary: Origin\r\n";
    }

    void close_current_client()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        close_current_client_locked();
    }

    void close_current_client_locked()
    {
        if (m_current_client != INVALID_SOCKET_VALUE)
            SOCKET_CLOSE(m_current_client);
        m_current_client = INVALID_SOCKET_VALUE;
        m_current_origin.clear();
        m_client_cv.notify_all();
    }

    bool send_http_response(int code, const char* status, const char* content_type, const std::string& body, bool include_cors, const char* extra_headers = NULL)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed || m_current_client == INVALID_SOCKET_VALUE)
        {
            Debug("[MCP] HTTP send failed: %s (closed=%d, client=%d)",
                m_closed ? "server closed" : "no client connected",
                m_closed, (int)m_current_client);
            return false;
        }

        std::string response =
            "HTTP/1.1 " + std::to_string(code) + " " + status + "\r\n";

        if (content_type)
        {
            response += "Content-Type: " + std::string(content_type) + "\r\n";
            response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        }

        if (include_cors)
            response += get_cors_origin_header();

        if (extra_headers)
            response += extra_headers;

        response +=
            "Connection: close\r\n"
            "\r\n" +
            body;

        size_t total_sent = 0;
        while (total_sent < response.length())
        {
            int sent = ::send(m_current_client, response.c_str() + total_sent,
                              (int)(response.length() - total_sent), 0);
            if (sent <= 0)
            {
                Error("[MCP] HTTP send() failed: %d", sent);
                close_current_client_locked();
                return false;
            }
            total_sent += sent;
        }

        std::string log_output = body.length() > 500 ? body.substr(0, 500) + "..." : body;
        Debug("[MCP] HTTP response sent (%d bytes): %d %s%s%s",
              (int)total_sent, code, status, body.empty() ? "" : ": ", log_output.c_str());

        close_current_client_locked();
        return true;
    }

    bool validate_and_reject_bearer_auth(const std::string& request, socket_t client)
    {
        if (m_auth_token.empty())
            return true;

        std::string authorization;
        if (extract_http_header(request, "Authorization", authorization) != 1)
        {
            Debug("[MCP] Rejecting request with missing or duplicate Authorization header");
            send_http_response(401, "Unauthorized", "text/plain", "401 Unauthorized", true, "WWW-Authenticate: Bearer\r\n");
            return false;
        }

        std::string expected = "Bearer " + m_auth_token;

        if (authorization == expected)
            return true;

        Debug("[MCP] Rejecting request with missing or invalid bearer token");
        send_http_response(401, "Unauthorized", "text/plain", "401 Unauthorized", true, "WWW-Authenticate: Bearer\r\n");
        return false;
    }

    bool validate_and_reject_http_access(const std::string& request, socket_t client)
    {
        std::string local_address;
        if (!get_socket_local_address(client, local_address))
        {
            Debug("[MCP] Rejecting request because the local socket address is unavailable");
            send_http_response(403, "Forbidden", "text/plain", "403 Forbidden", false);
            return false;
        }

        std::string host;
        if (extract_http_header(request, "Host", host) != 1 || !is_allowed_host(host, local_address))
        {
            Debug("[MCP] Rejecting request with missing, duplicate, or foreign Host: %s", host.c_str());
            send_http_response(403, "Forbidden", "text/plain", "403 Forbidden", false);
            return false;
        }

        std::string origin;
        int origin_count = extract_http_header(request, "Origin", origin);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_current_origin.clear();
            if (origin_count == 1 && is_allowed_origin(origin, local_address))
                m_current_origin = origin;
        }

        if (origin_count > 1 || (origin_count == 1 && m_current_origin.empty()))
        {
            Debug("[MCP] Rejecting request with duplicate or foreign Origin: %s", origin.c_str());
            send_http_response(403, "Forbidden", "text/plain", "403 Forbidden", false);
            return false;
        }

        return true;
    }

    bool validate_and_reject_invalid_request(const std::string& request, socket_t client, bool require_auth)
    {
        if (!validate_and_reject_http_access(request, client))
            return false;

        std::string path = extract_http_path(request);
        if (path == MCP_HTTP_ENDPOINT_PATH)
        {
            if (require_auth && !validate_and_reject_bearer_auth(request, client))
                return false;

            return true;
        }

        Debug("[MCP] Rejecting request to invalid path: %s", path.c_str());
        send_http_response(404, "Not Found", "text/plain", "404 Not Found", true);
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
            socket_len_t client_len = sizeof(client_addr);
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
                m_current_origin.clear();
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
                        Debug("[MCP] HTTP headers:\n%s", sanitize_http_headers_for_debug(request.substr(0, header_end)).c_str());

                        // Check if this is an OPTIONS request (CORS preflight)
                        if (request.find("OPTIONS ") == 0)
                        {
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
                close_current_client();
                continue; // Wait for next connection
            }

            // Handle OPTIONS request (CORS preflight)
            if (is_options)
            {
                if (!validate_and_reject_invalid_request(request, client, false))
                    continue;

                Debug("[MCP] HTTP OPTIONS request (CORS preflight)");

                send_http_response(204, "No Content", NULL, "", true,
                                   "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                                   "Access-Control-Allow-Headers: Authorization, Content-Type, Accept, MCP-Session-Id, MCP-Protocol-Version\r\n");
                continue; // Wait for next connection
            }

            // Handle POST with body (normal MCP request)
            if (!read_error && header_end > 0 && content_length > 0)
            {
                if (!validate_and_reject_invalid_request(request, client, true))
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
                if (!validate_and_reject_invalid_request(request, client, true))
                    continue;
                
                Debug("[MCP] HTTP GET request (SSE not supported, responding 405)");

                const std::string body = "{\"error\":\"SSE streaming not supported by this server\"}";
                send_http_response(405, "Method Not Allowed", "application/json", body, true,
                                   "Allow: POST, OPTIONS\r\n");
                continue; // Wait for next request
            }

            // Handle POST without body - VS Code MCP client doesn't do health checks this way
            // According to spec, all MCP messages should have a JSON-RPC body
            if (!read_error && header_end > 0 && content_length == 0 && request.find("POST ") == 0)
            {
                if (!validate_and_reject_invalid_request(request, client, true))
                    continue;

                Debug("[MCP] HTTP POST without body (non-standard, closing connection)");

                close_current_client();
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

                close_current_client();
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
        m_current_origin.clear();

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
    int m_port;
    std::string m_bind_address;
    std::string m_auth_token;
    std::string m_current_origin;
    socket_t m_server_socket;
    socket_t m_current_client;
};

#endif /* MCP_TRANSPORT_H */
