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
#define MCP_HTTP_MAX_HEADER_SIZE (64 * 1024)
#define MCP_MAX_MESSAGE_SIZE (4 * 1024 * 1024)
#define MCP_HTTP_MAX_BODY_SIZE MCP_MAX_MESSAGE_SIZE
#define MCP_HTTP_RECEIVE_TIMEOUT_MS 5000
#define MCP_HTTP_SEND_TIMEOUT_MS 5000
#define MCP_STDIO_POLL_TIMEOUT_MS 100
#define MCP_PROTOCOL_VERSION "2025-11-25"

#include <string>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "log.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <conio.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    typedef int socket_len_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_CLOSE(s) closesocket(s)
    #define SOCKET_SHUTDOWN(s) shutdown(s, SD_BOTH)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int socket_t;
    typedef socklen_t socket_len_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_CLOSE(s) ::close(s)
    #define SOCKET_SHUTDOWN(s) ::shutdown(s, SHUT_RDWR)
#endif

class McpTransportInterface
{
public:
    virtual ~McpTransportInterface() {}
    virtual bool send(const std::string& jsonLine) = 0;
    virtual bool acknowledge_notification() = 0;
    virtual bool reject_notification() = 0;
    virtual bool validate_protocol_version(const std::string& method) = 0;
    virtual void set_protocol_version(const std::string& version) = 0;
    virtual bool recv(std::string& jsonLine) = 0;
    virtual void close() = 0;
};

class StdioTransport : public McpTransportInterface
{
public:
    StdioTransport()
    {
        m_closed.store(false);
        m_input_eof = false;
    }
    ~StdioTransport() {}

    bool send(const std::string& jsonLine)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed.load())
            return false;
        std::cout << jsonLine << "\n";
        std::cout.flush();
        return true;
    }

    bool acknowledge_notification()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_closed.load();
    }

    bool reject_notification()
    {
        return acknowledge_notification();
    }

    bool validate_protocol_version(const std::string& method)
    {
        (void)method;
        return true;
    }

    void set_protocol_version(const std::string& version)
    {
        (void)version;
    }

    bool recv(std::string& jsonLine)
    {
        while (!m_closed.load())
        {
            size_t line_end = m_input_buffer.find('\n');
            if (line_end != std::string::npos)
            {
                jsonLine = m_input_buffer.substr(0, line_end);
                m_input_buffer.erase(0, line_end + 1);
                if (!jsonLine.empty() && jsonLine[jsonLine.length() - 1] == '\r')
                    jsonLine.erase(jsonLine.length() - 1);
                return true;
            }

            if (m_input_eof)
            {
                if (m_input_buffer.empty())
                    return false;

                jsonLine = m_input_buffer;
                m_input_buffer.clear();
                return true;
            }

            char buffer[4096];
            int received = read_stdin(buffer, sizeof(buffer));
            if (received > 0)
            {
                const char* newline = (const char*)memchr(buffer, '\n', (size_t)received);
                size_t bytes_before_newline = newline ? (size_t)(newline - buffer) : (size_t)received;
                if (bytes_before_newline > MCP_MAX_MESSAGE_SIZE ||
                    m_input_buffer.size() > MCP_MAX_MESSAGE_SIZE - bytes_before_newline)
                {
                    return false;
                }
                m_input_buffer.append(buffer, received);
            }
            else if (received == 0)
                m_input_eof = true;
            else if (received != -2)
                return false;
        }

        return false;
    }

    void close()
    {
        m_closed.store(true);
    }

private:
    int read_stdin(char* buffer, int length)
    {
#ifdef _WIN32
        HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
        if (input == NULL || input == INVALID_HANDLE_VALUE)
            return -1;

        DWORD input_type = GetFileType(input);
        if (input_type == FILE_TYPE_PIPE)
        {
            DWORD available = 0;
            if (!PeekNamedPipe(input, NULL, 0, NULL, &available, NULL))
            {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                    return 0;
                return -1;
            }

            if (available == 0)
            {
                Sleep(MCP_STDIO_POLL_TIMEOUT_MS);
                return -2;
            }

            DWORD received = 0;
            DWORD bytes_to_read = available < (DWORD)length ? available : (DWORD)length;
            if (!ReadFile(input, buffer, bytes_to_read, &received, NULL))
                return GetLastError() == ERROR_BROKEN_PIPE ? 0 : -1;
            return (int)received;
        }

        if (input_type == FILE_TYPE_CHAR)
        {
            DWORD wait_result = WaitForSingleObject(input, MCP_STDIO_POLL_TIMEOUT_MS);
            if (wait_result == WAIT_TIMEOUT)
                return -2;
            if (wait_result != WAIT_OBJECT_0)
                return -1;

            int received = 0;
            while (received < length && _kbhit())
            {
                int character = _getch();
                if (character == '\r')
                    character = '\n';
                buffer[received++] = (char)character;
                if (character == '\n')
                    break;
            }
            return received > 0 ? received : -2;
        }

        DWORD received = 0;
        if (!ReadFile(input, buffer, (DWORD)length, &received, NULL))
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                return 0;
            return -1;
        }
        return (int)received;
#else
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);

        struct timeval timeout;
        timeout.tv_sec = MCP_STDIO_POLL_TIMEOUT_MS / 1000;
        timeout.tv_usec = (MCP_STDIO_POLL_TIMEOUT_MS % 1000) * 1000;

        int ready = select(STDIN_FILENO + 1, &read_set, NULL, NULL, &timeout);
        if (ready == 0)
            return -2;
        if (ready < 0)
            return errno == EINTR ? -2 : -1;

        ssize_t received = ::read(STDIN_FILENO, buffer, (size_t)length);
        if (received < 0)
            return (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) ? -2 : -1;
        return (int)received;
#endif
    }

    std::mutex m_mutex;
    std::atomic<bool> m_closed;
    std::string m_input_buffer;
    bool m_input_eof;
};

class HttpTransport : public McpTransportInterface
{
public:
    HttpTransport(const std::string& address, int port)
    {
        m_closed.store(false);
        m_port = port;
        m_bind_address = address.empty() ? "127.0.0.1" : address;
        if (m_bind_address == "localhost")
            m_bind_address = "127.0.0.1";
        const char* auth_token = getenv(MCP_HTTP_AUTH_ENV);
        if (auth_token && auth_token[0])
            m_auth_token = auth_token;
        m_server_socket = INVALID_SOCKET_VALUE;
        m_current_client = INVALID_SOCKET_VALUE;
        m_pending_protocol_version_count = 0;
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

    bool reject_notification()
    {
        return send_http_response(400, "Bad Request", NULL, "", true);
    }

    bool validate_protocol_version(const std::string& method)
    {
        if (method == "initialize")
        {
            if (m_pending_protocol_version_count <= 1)
                return true;
        }
        else if (validate_protocol_version_header(false))
        {
            return true;
        }

        send_http_response(400, "Bad Request", NULL, "", true);
        return false;
    }

    void set_protocol_version(const std::string& version)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_protocol_version = version;
    }

    bool extract_http_method(const std::string& request, std::string& method)
    {
        method.clear();
        size_t line_end = request.find("\r\n");
        size_t method_end = request.find(' ');
        if (line_end == std::string::npos || method_end == std::string::npos || method_end == 0 || method_end >= line_end)
            return false;

        size_t path_end = request.find(' ', method_end + 1);
        if (path_end == std::string::npos || path_end <= method_end + 1 || path_end >= line_end)
            return false;

        if (request.compare(path_end + 1, line_end - path_end - 1, "HTTP/1.1") != 0)
            return false;

        method = request.substr(0, method_end);
        return true;
    }

    std::string extract_http_path(const std::string& request)
    {
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

    bool parse_content_length(const std::string& value, size_t& content_length)
    {
        if (value.empty())
            return false;

        size_t parsed = 0;
        for (size_t i = 0; i < value.length(); i++)
        {
            char character = value[i];
            if (character < '0' || character > '9')
                return false;

            size_t digit = (size_t)(character - '0');
            if (parsed > (((size_t)-1) - digit) / 10)
                return false;
            parsed = parsed * 10 + digit;
        }

        content_length = parsed;
        return true;
    }

    bool configure_client_socket_timeouts(socket_t client)
    {
#ifdef _WIN32
        DWORD receive_timeout = MCP_HTTP_RECEIVE_TIMEOUT_MS;
        DWORD send_timeout = MCP_HTTP_SEND_TIMEOUT_MS;
        return setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&receive_timeout, sizeof(receive_timeout)) == 0 &&
               setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, (const char*)&send_timeout, sizeof(send_timeout)) == 0;
#else
        struct timeval receive_timeout;
        receive_timeout.tv_sec = MCP_HTTP_RECEIVE_TIMEOUT_MS / 1000;
        receive_timeout.tv_usec = (MCP_HTTP_RECEIVE_TIMEOUT_MS % 1000) * 1000;

        struct timeval send_timeout;
        send_timeout.tv_sec = MCP_HTTP_SEND_TIMEOUT_MS / 1000;
        send_timeout.tv_usec = (MCP_HTTP_SEND_TIMEOUT_MS % 1000) * 1000;

        return setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &receive_timeout, sizeof(receive_timeout)) == 0 &&
               setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) == 0;
#endif
    }

    bool is_receive_timeout_error()
    {
#ifdef _WIN32
        int error = WSAGetLastError();
        return error == WSAETIMEDOUT || error == WSAEWOULDBLOCK;
#else
        return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
    }

    bool validate_protocol_version_header(bool allow_uninitialized)
    {
        std::string protocol_version;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            protocol_version = m_protocol_version;
        }

        if (protocol_version.empty())
            return allow_uninitialized;

        return m_pending_protocol_version_count == 1 && m_pending_protocol_version == protocol_version;
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
        {
            SOCKET_SHUTDOWN(m_current_client);
            SOCKET_CLOSE(m_current_client);
        }
        m_current_client = INVALID_SOCKET_VALUE;
        m_current_origin.clear();
        m_client_cv.notify_all();
    }

    bool send_http_response(int code, const char* status, const char* content_type, const std::string& body, bool include_cors, const char* extra_headers = NULL)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed.load() || m_current_client == INVALID_SOCKET_VALUE)
        {
            Debug("[MCP] HTTP send failed: %s (closed=%d, client=%d)",
            m_closed.load() ? "server closed" : "no client connected",
            m_closed.load(), (int)m_current_client);
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

            if (m_closed.load())
            {
                close_current_client_locked();
                return false;
            }
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
        while (!m_closed.load())
        {
            socket_t server_socket = INVALID_SOCKET_VALUE;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_client_cv.wait(lock, [this]
                {
                    return m_closed.load() || m_current_client == INVALID_SOCKET_VALUE;
                });

                if (m_closed.load())
                {
                    Debug("[MCP] Server closed, stopping recv loop");
                    return false;
                }

                server_socket = m_server_socket;
                if (server_socket == INVALID_SOCKET_VALUE)
                    return false;
            }

            struct sockaddr_in client_addr;
            socket_len_t client_len = sizeof(client_addr);
            socket_t client = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

            if (client == INVALID_SOCKET_VALUE)
            {
                if (m_closed.load())
                {
                    Debug("[MCP] Server closed, stopping recv loop");
                    return false;
                }
                Debug("[MCP] accept() failed, retrying");
                continue;
            }

            Log("[MCP] HTTP client connected");

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_closed.load())
                {
                    SOCKET_SHUTDOWN(client);
                    SOCKET_CLOSE(client);
                    return false;
                }
                m_current_client = client;
                m_current_origin.clear();
            }
            m_pending_protocol_version.clear();
            m_pending_protocol_version_count = 0;

            if (!configure_client_socket_timeouts(client))
            {
                Error("[MCP] Failed to configure HTTP client socket timeouts");
                close_current_client();
                continue;
            }

            std::string request;
            char buffer[4096];
            int header_end = -1;
            size_t content_length = 0;
            std::string http_method;
            bool read_error = false;
            bool connection_closed = false;
            bool request_timed_out = false;
            bool header_too_large = false;
            bool body_too_large = false;
            bool invalid_framing = false;
            bool request_line_valid = false;

            while (true)
            {
                int received = ::recv(client, buffer, sizeof(buffer), 0);
                if (received <= 0)
                {
                    read_error = true;
                    connection_closed = (received == 0);
                    request_timed_out = (received < 0) && is_receive_timeout_error();
                    Debug("[MCP] HTTP recv %s (received=%d)", 
                            connection_closed ? "connection closed by client" : (request_timed_out ? "timeout" : "error"),
                            received);
                    break;
                }

                request.append(buffer, received);

                if (header_end == -1)
                {
                    size_t pos = request.find("\r\n\r\n");
                    if (pos != std::string::npos)
                    {
                        header_end = (int)(pos + 4);
                        request_line_valid = extract_http_method(request, http_method);

                        if ((size_t)header_end > MCP_HTTP_MAX_HEADER_SIZE)
                        {
                            header_too_large = true;
                            break;
                        }

                        Debug("[MCP] HTTP headers:\n%s", sanitize_http_headers_for_debug(request.substr(0, header_end)).c_str());

                        m_pending_protocol_version_count = extract_http_header(request, "MCP-Protocol-Version", m_pending_protocol_version);

                        if (!request_line_valid)
                        {
                            invalid_framing = true;
                            break;
                        }

                        std::string transfer_encoding;
                        if (extract_http_header(request, "Transfer-Encoding", transfer_encoding) != 0)
                        {
                            invalid_framing = true;
                            break;
                        }

                        if (http_method != "POST")
                        {
                            content_length = 0;
                            break;
                        }

                        std::string content_length_value;
                        if (extract_http_header(request, "Content-Length", content_length_value) != 1 ||
                            !parse_content_length(content_length_value, content_length) || content_length == 0)
                        {
                            invalid_framing = true;
                            break;
                        }

                        if (content_length > MCP_HTTP_MAX_BODY_SIZE)
                        {
                            body_too_large = true;
                            break;
                        }
                    }
                    else if (request.length() > MCP_HTTP_MAX_HEADER_SIZE)
                    {
                        header_too_large = true;
                        break;
                    }
                }

                if (header_end > 0 && http_method == "POST" && !invalid_framing && !body_too_large)
                {
                    size_t body_length = request.length() - (size_t)header_end;
                    if (body_length >= content_length)
                        break;
                }
            }

            if (connection_closed)
            {
                Log("[MCP] Client disconnected, cleaning up");
                close_current_client();
                continue;
            }

            if (request_timed_out || (read_error && !connection_closed))
            {
                close_current_client();
                continue;
            }

            if (header_end <= 0)
            {
                close_current_client();
                continue;
            }

            if (!request_line_valid)
            {
                if (validate_and_reject_http_access(request, client))
                    send_http_response(400, "Bad Request", "text/plain", "400 Bad Request", true);
                continue;
            }

            bool require_auth = (http_method != "OPTIONS");
            if (!validate_and_reject_invalid_request(request, client, require_auth))
                continue;

            if (header_too_large)
            {
                send_http_response(431, "Request Header Fields Too Large", "text/plain", "431 Request Header Fields Too Large", true);
                continue;
            }

            if (body_too_large)
            {
                send_http_response(413, "Content Too Large", "text/plain", "413 Content Too Large", true);
                continue;
            }

            if (invalid_framing)
            {
                send_http_response(400, "Bad Request", "text/plain", "400 Bad Request", true);
                continue;
            }

            if (http_method == "OPTIONS")
            {
                Debug("[MCP] HTTP OPTIONS request (CORS preflight)");

                send_http_response(204, "No Content", NULL, "", true,
                                   "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                                   "Access-Control-Allow-Headers: Authorization, Content-Type, Accept, MCP-Session-Id, MCP-Protocol-Version\r\n");
                continue;
            }

            if (http_method == "POST")
            {
                jsonLine = request.substr(header_end, content_length);
                Debug("[MCP] HTTP POST request received (%d bytes): %s", 
                    (int)jsonLine.length(), 
                    jsonLine.substr(0, 100).c_str());
                return true;
            }

            if (http_method == "GET" && !validate_protocol_version_header(true))
            {
                send_http_response(400, "Bad Request", NULL, "", true);
                continue;
            }

            Debug("[MCP] HTTP %s request not supported, responding 405", http_method.c_str());

            if (http_method == "HEAD")
            {
                send_http_response(405, "Method Not Allowed", NULL, "", true,
                                   "Allow: POST, OPTIONS\r\n");
            }
            else if (http_method == "GET")
            {
                const std::string body = "{\"error\":\"SSE streaming not supported by this server\"}";
                send_http_response(405, "Method Not Allowed", "application/json", body, true,
                                   "Allow: POST, OPTIONS\r\n");
            }
            else
            {
                const std::string body = "{\"error\":\"HTTP method not supported by this server\"}";
                send_http_response(405, "Method Not Allowed", "application/json", body, true,
                                   "Allow: POST, OPTIONS\r\n");
            }

            continue;
        }

        return false;
    }

    void close()
    {
        if (m_closed.exchange(true))
            return;

        std::lock_guard<std::mutex> lock(m_mutex);
        close_current_client_locked();

        if (m_server_socket != INVALID_SOCKET_VALUE)
        {
            SOCKET_SHUTDOWN(m_server_socket);
            SOCKET_CLOSE(m_server_socket);
            m_server_socket = INVALID_SOCKET_VALUE;
        }

        m_client_cv.notify_all();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_client_cv;
    std::atomic<bool> m_closed;
    int m_port;
    std::string m_bind_address;
    std::string m_auth_token;
    std::string m_current_origin;
    std::string m_protocol_version;
    std::string m_pending_protocol_version;
    int m_pending_protocol_version_count;
    socket_t m_server_socket;
    socket_t m_current_client;
};

#endif /* MCP_TRANSPORT_H */
