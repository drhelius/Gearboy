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

    void SetTransportMode(McpTransportMode mode, int tcp_port = 7777)
    {
        m_transport_mode = mode;
        m_tcp_port = tcp_port;
    }

    void Start()
    {
        if (m_server && m_server->IsRunning())
            return;

        m_commandQueue.Clear();
        m_responseQueue.Reset();

        McpTransportInterface* transport = NULL;
        if (m_transport_mode == MCP_TRANSPORT_TCP)
        {
            g_mcp_stdio_mode = false;
            Log("[MCP] Starting HTTP transport on port %d", m_tcp_port);
            transport = new HttpTransport(m_tcp_port);
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

        DebugCommand* cmd = NULL;
        while ((cmd = m_commandQueue.Pop()) != NULL)
        {
            DebugResponse* resp = new DebugResponse();
            resp->requestId = cmd->requestId;
            resp->isError = false;

            resp->result = m_server->ExecuteCommand(cmd->toolName, cmd->arguments);

            if (resp->result.contains("error"))
            {
                resp->isError = true;
                resp->errorCode = -32603;
                resp->errorMessage = resp->result["error"];
            }

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
        }
    }

private:
    DebugAdapter* m_debugAdapter;
    McpServer* m_server;
    CommandQueue m_commandQueue;
    ResponseQueue m_responseQueue;
    McpTransportMode m_transport_mode;
    int m_tcp_port;
    std::vector<DelayedButtonRelease> m_delayedReleases;
};

#endif /* MCP_MANAGER_H */
