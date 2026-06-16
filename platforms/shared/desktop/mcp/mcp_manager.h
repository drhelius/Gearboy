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

#ifndef MCP_MANAGER_H
#define MCP_MANAGER_H

#include "mcp_server.h"
#include "mcp_transport.h"
#include "mcp_debug_adapter.h"
#include "emu.h"
#include <vector>

extern bool g_mcp_stdio_mode;

enum McpTransportMode
{
    MCP_TRANSPORT_STDIO,
    MCP_TRANSPORT_TCP
};

struct DelayedButtonRelease
{
    int player;
    std::string button;
    u64 release_at_cycle;
};

class McpManager
{
public:
    McpManager()
    {
        m_debugAdapter = NULL;
        m_server = NULL;
        m_transport_mode = MCP_TRANSPORT_STDIO;
        m_tcp_port = 7777;
        m_tcp_address = "127.0.0.1";
        m_pending_media_load = false;
        m_pending_media_load_request_id = 0;
    }

    ~McpManager()
    {
        Stop();
        SafeDelete(m_debugAdapter);
    }

    void Init(GearboyCore* core)
    {
        m_debugAdapter = new DebugAdapter(core);
    }

    void SetTransportMode(McpTransportMode mode, int tcp_port = 7777, const char* tcp_address = "127.0.0.1")
    {
        m_transport_mode = mode;
        m_tcp_port = tcp_port;
        m_tcp_address = (tcp_address && tcp_address[0]) ? tcp_address : "127.0.0.1";
    }

    void Start()
    {
        if (m_server && m_server->IsRunning())
            return;

        m_commandQueue.Clear();
        m_responseQueue.Reset();
        m_pending_media_load = false;
        m_pending_media_load_file_path.clear();

        McpTransportInterface* transport = NULL;
        if (m_transport_mode == MCP_TRANSPORT_TCP)
        {
            g_mcp_stdio_mode = false;
            Log("[MCP] Starting HTTP transport on %s:%d", m_tcp_address.c_str(), m_tcp_port);
            transport = new HttpTransport(m_tcp_address, m_tcp_port);
        }
        else
        {
            g_mcp_stdio_mode = true;
            transport = new StdioTransport();
        }

        m_server = new McpServer(
            transport,
            *m_debugAdapter,
            m_commandQueue,
            m_responseQueue
        );
        m_server->Start();
    }

    void Stop()
    {
        if (m_server)
        {
            m_server->Stop();

            if (m_server->GetTransport())
                m_server->GetTransport()->close();

            SafeDelete(m_server);
        }
    }

    bool IsRunning() const
    {
        return m_server && m_server->IsRunning();
    }

    int GetTransportMode() const
    {
        return (int)m_transport_mode;
    }

    void PumpCommands(GearboyCore* core)
    {
        u64 current_cycles = core->GetMasterClockCycles();

        for (size_t i = 0; i < m_delayedReleases.size(); )
        {
            if (current_cycles >= m_delayedReleases[i].release_at_cycle)
            {
                m_debugAdapter->ControllerButton(m_delayedReleases[i].player, m_delayedReleases[i].button, "release");
                m_delayedReleases.erase(m_delayedReleases.begin() + i);
            }
            else
                i++;
        }

        if (m_pending_media_load)
        {
            if (m_debugAdapter->IsMediaLoading())
                return;

            DebugResponse* resp = new DebugResponse();
            resp->requestId = m_pending_media_load_request_id;
            resp->isError = false;
            resp->result = m_debugAdapter->FinishLoadMedia(m_pending_media_load_file_path);

            update_response_error(resp);

            m_pending_media_load = false;
            m_pending_media_load_file_path.clear();
            m_responseQueue.Push(resp);
        }

        DebugCommand* cmd = NULL;
        while ((cmd = m_commandQueue.Pop()) != NULL)
        {
            bool was_idle = emu_is_debug_idle();

            if (is_load_media_command(cmd->toolName))
            {
                DebugResponse* resp = new DebugResponse();
                resp->requestId = cmd->requestId;
                resp->isError = false;

                std::string file_path = cmd->arguments.value("file_path", "");
                resp->result = m_debugAdapter->StartLoadMedia(file_path);

                if (resp->result.contains("error"))
                {
                    update_response_error(resp);
                    m_responseQueue.Push(resp);
                }
                else
                {
                    m_pending_media_load = true;
                    m_pending_media_load_request_id = resp->requestId;
                    m_pending_media_load_file_path = file_path;
                    SafeDelete(resp);
                }

                SafeDelete(cmd);
                break;
            }

            DebugResponse* resp = new DebugResponse();
            resp->requestId = cmd->requestId;
            resp->isError = false;

            resp->result = m_server->ExecuteCommand(cmd->toolName, cmd->arguments);

            update_response_error(resp);

            if (resp->result.contains("__delayed_release") && resp->result["__delayed_release"] == true)
            {
                DelayedButtonRelease release;
                release.player = resp->result["player"];
                release.button = resp->result["button"];
                release.release_at_cycle = core->GetMasterClockCycles() + 3000000; // around 10 frames
                m_delayedReleases.push_back(release);

                resp->result.erase("__delayed_release");
            }

            m_responseQueue.Push(resp);
            SafeDelete(cmd);

            if (was_idle && !emu_is_debug_idle())
                break;
        }
    }

private:
    bool is_load_media_command(const std::string& tool_name) const
    {
        std::string normalized_tool = tool_name;
        size_t pos = 0;
        while ((pos = normalized_tool.find('.', pos)) != std::string::npos)
        {
            normalized_tool[pos] = '_';
            pos++;
        }

        return normalized_tool == "load_media";
    }

    void update_response_error(DebugResponse* resp)
    {
        if (!resp->result.contains("error"))
            return;

        resp->isError = true;
        resp->errorCode = -32603;
        resp->errorMessage = resp->result["error"];
    }

    DebugAdapter* m_debugAdapter;
    McpServer* m_server;
    CommandQueue m_commandQueue;
    ResponseQueue m_responseQueue;
    McpTransportMode m_transport_mode;
    int m_tcp_port;
    std::string m_tcp_address;
    bool m_pending_media_load;
    int64_t m_pending_media_load_request_id;
    std::string m_pending_media_load_file_path;
    std::vector<DelayedButtonRelease> m_delayedReleases;
};

#endif /* MCP_MANAGER_H */
