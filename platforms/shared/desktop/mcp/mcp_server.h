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

#ifndef MCP_SERVER_H
#define MCP_SERVER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <map>
#include "json.hpp"
#include "mcp_transport.h"
#include "mcp_debug_adapter.h"

using json = nlohmann::json;

struct ResourceInfo
{
    std::string uri;
    std::string title;
    std::string description;
    std::string mimeType;
    std::string category;
    std::string filePath;
};

struct DebugCommand
{
    int64_t requestId;
    std::string toolName;
    json arguments;
};

struct DebugResponse
{
    int64_t requestId;
    bool isError;
    int errorCode;
    std::string errorMessage;
    json result;
};

class CommandQueue
{
public:
    void Push(DebugCommand* cmd)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(cmd);
    }

    DebugCommand* Pop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return NULL;
        DebugCommand* cmd = m_queue.front();
        m_queue.pop();
        return cmd;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
        {
            SafeDelete(m_queue.front());
            m_queue.pop();
        }
    }

private:
    std::queue<DebugCommand*> m_queue;
    std::mutex m_mutex;
};

class ResponseQueue
{
public:
    void Push(DebugResponse* resp)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(resp);
        m_cv.notify_one();
    }

    DebugResponse* WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_queue.empty() || !m_running; });

        if (m_queue.empty())
            return NULL;

        DebugResponse* resp = m_queue.front();
        m_queue.pop();
        return resp;
    }

    void Stop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
        m_cv.notify_all();
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
        {
            SafeDelete(m_queue.front());
            m_queue.pop();
        }
        m_running = true;
    }

private:
    std::queue<DebugResponse*> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_running = true;
};

class McpServer
{
public:
    McpServer(McpTransportInterface* transport,
              DebugAdapter& debugAdapter,
              CommandQueue& commandQueue,
              ResponseQueue& responseQueue)
        : m_debugAdapter(debugAdapter),
          m_commandQueue(commandQueue),
          m_responseQueue(responseQueue)
    {
        m_transport = transport;
        m_running = false;
        m_initialized = false;
    }

    ~McpServer()
    {
        Stop();
        SafeDelete(m_transport);
    }

    void Start()
    {
        if (m_running.load())
            return;

        LoadResources();
        m_running.store(true);
        m_thread = std::thread(&McpServer::Run, this);
    }

    void Stop()
    {
        m_running.store(false);
        m_responseQueue.Stop();

        // Detach the thread instead of joining to avoid blocking on stdin read
        // The thread will exit naturally when recv() returns or on next iteration
        if (m_thread.joinable())
            m_thread.detach();
    }

    bool IsRunning() const
    {
        return m_running.load();
    }

    McpTransportInterface* GetTransport() const
    {
        return m_transport;
    }

    json ExecuteCommand(const std::string& toolName, const json& arguments);

    void ReaderLoop();

private:
    void Run();
    void HandleLine(const std::string& line);
    void HandleInitialize(const json& request);
    void HandleToolsList(const json& request);
    void HandleToolsCall(const json& request);
    void HandleResourcesList(const json& request);
    void HandleResourcesRead(const json& request);

    void LoadResources();
    void LoadResourcesFromCategory(const std::string& category, const std::string& tocPath);
    std::string ReadFileContents(const std::string& filePath);

    void SendResponse(const json& response);
    void SendError(int64_t id, int code, const std::string& message, const json& data = json::object());

    McpTransportInterface* m_transport;
    DebugAdapter& m_debugAdapter;
    CommandQueue& m_commandQueue;
    ResponseQueue& m_responseQueue;
    std::thread m_thread;
    std::atomic<bool> m_running;
    bool m_initialized;
    std::vector<ResourceInfo> m_resources;
    std::map<std::string, ResourceInfo> m_resourceMap;
};

#endif /* MCP_SERVER_H */
