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
#include <string>
#include <algorithm>
#include <cctype>

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

enum McpInputMacroStepType
{
    MCP_INPUT_MACRO_STEP_PRESS,
    MCP_INPUT_MACRO_STEP_RELEASE,
    MCP_INPUT_MACRO_STEP_WAIT
};

struct McpInputMacroStep
{
    McpInputMacroStepType type;
    int player;
    std::string button;
    int frames;
};

struct McpInputMacroState
{
    bool active;
    int64_t request_id;
    std::vector<McpInputMacroStep> steps;
    size_t step_index;
    bool waiting;
    u64 wait_target_frame;
    int command_count;
    int frames_waited;
    bool restore_pause;
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
        reset_input_macro();
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
        reset_input_macro();

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

            if (m_inputMacro.active)
            {
                pump_input_macro(core);
                return;
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

            if (is_controller_macro_command(cmd->toolName))
            {
                DebugResponse* resp = new DebugResponse();
                resp->requestId = cmd->requestId;
                resp->isError = false;
                resp->result = start_input_macro(cmd->arguments, cmd->requestId);

                if (resp->result.contains("error"))
                {
                    update_response_error(resp);
                    m_responseQueue.Push(resp);
                }
                else
                {
                    SafeDelete(resp);
                    pump_input_macro(core);
                }

                SafeDelete(cmd);
                break;
            }

            DebugResponse* resp = new DebugResponse();
            resp->requestId = cmd->requestId;
            resp->isError = false;

            resp->result = m_server->ExecuteCommand(cmd->toolName, cmd->arguments);

            if (is_get_input_state_command(cmd->toolName))
                append_input_runtime_state(resp->result);

            update_response_error(resp);
            handle_controller_side_effects(core, resp->result);

            m_responseQueue.Push(resp);
            SafeDelete(cmd);

            if (was_idle && !emu_is_debug_idle())
                break;
        }
    }

private:
    std::string normalize_tool_name(const std::string& tool_name) const
    {
        std::string normalized_tool = tool_name;
        size_t pos = 0;
        while ((pos = normalized_tool.find('.', pos)) != std::string::npos)
        {
            normalized_tool[pos] = '_';
            pos++;
        }

        return normalized_tool;
    }

    bool is_load_media_command(const std::string& tool_name) const
    {
        return normalize_tool_name(tool_name) == "load_media";
    }

    bool is_controller_macro_command(const std::string& tool_name) const
    {
        return normalize_tool_name(tool_name) == "controller_macro";
    }

    bool is_get_input_state_command(const std::string& tool_name) const
    {
        return normalize_tool_name(tool_name) == "get_input_state";
    }

    void append_input_runtime_state(json& result) const
    {
        if (result.contains("error"))
            return;

        json pending_releases = json::array();
        for (size_t i = 0; i < m_delayedReleases.size(); i++)
        {
            pending_releases.push_back({
                {"player", m_delayedReleases[i].player},
                {"button", m_delayedReleases[i].button}
            });
        }

        result["pending_releases"] = pending_releases;
    }

    void update_response_error(DebugResponse* resp)
    {
        if (!resp->result.contains("error"))
            return;

        resp->isToolError = true;
        resp->errorMessage = resp->result["error"];
    }

    void reset_input_macro()
    {
        m_inputMacro.active = false;
        m_inputMacro.request_id = 0;
        m_inputMacro.steps.clear();
        m_inputMacro.step_index = 0;
        m_inputMacro.waiting = false;
        m_inputMacro.wait_target_frame = 0;
        m_inputMacro.command_count = 0;
        m_inputMacro.frames_waited = 0;
        m_inputMacro.restore_pause = false;
    }

    json start_input_macro(const json& arguments, int64_t request_id)
    {
        json result;

        if (emu_is_empty())
        {
            result["error"] = "No media loaded";
            return result;
        }

        if (arguments.contains("player") && !arguments["player"].is_number_integer())
        {
            result["error"] = "player must be an integer";
            return result;
        }

        int default_player = arguments.value("player", 1);
        if (default_player != 1)
        {
            result["error"] = "Invalid player number (must be 1)";
            return result;
        }

        if (!arguments.contains("commands") || !arguments["commands"].is_array())
        {
            result["error"] = "commands array is required";
            return result;
        }

        const json& commands = arguments["commands"];
        if (commands.empty())
        {
            result["error"] = "commands array must not be empty";
            return result;
        }

        reset_input_macro();
        m_inputMacro.request_id = request_id;
        m_inputMacro.command_count = (int)commands.size();
        m_inputMacro.restore_pause = emu_is_paused() && !emu_is_debug_idle();

        for (size_t i = 0; i < commands.size(); i++)
        {
            std::string error;
            if (!append_input_macro_command(commands[i], default_player, error))
            {
                reset_input_macro();
                result["error"] = "Invalid macro command " + std::to_string((int)i) + ": " + error;
                return result;
            }
        }

        m_inputMacro.active = true;

        result["success"] = true;
        result["pending"] = true;
        result["commands"] = m_inputMacro.command_count;
        result["steps"] = (int)m_inputMacro.steps.size();
        return result;
    }

    bool append_input_macro_command(const json& command, int default_player, std::string& error)
    {
        if (!command.is_object())
        {
            error = "command must be an object";
            return false;
        }

        int action_count = 0;
        if (command.contains("tap")) action_count++;
        if (command.contains("press")) action_count++;
        if (command.contains("release")) action_count++;
        if (command.contains("wait")) action_count++;

        if (action_count != 1)
        {
            error = "command must contain exactly one of tap, press, release, or wait";
            return false;
        }

        for (json::const_iterator it = command.begin(); it != command.end(); ++it)
        {
            if (it.key() != "tap" && it.key() != "press" && it.key() != "release" &&
                it.key() != "wait" && it.key() != "player")
            {
                error = "unknown property: " + it.key();
                return false;
            }
        }

        if (command.contains("player") && !command["player"].is_number_integer())
        {
            error = "player must be an integer";
            return false;
        }

        int player = command.value("player", default_player);
        if (player != 1)
        {
            error = "player must be 1";
            return false;
        }

        if (command.contains("wait"))
        {
            if (!command["wait"].is_number_integer())
            {
                error = "wait must be an integer frame count";
                return false;
            }

            int frames = command["wait"].get<int>();
            if (frames < 1 || frames > 1000)
            {
                error = "wait must be 1-1000 frames";
                return false;
            }

            append_wait_step(frames);
            return true;
        }

        const char* action = command.contains("tap") ? "tap" : (command.contains("press") ? "press" : "release");
        if (!command[action].is_string())
        {
            error = std::string(action) + " must be a button string";
            return false;
        }

        std::string button = command[action];

        if (!is_valid_button_name(button))
        {
            error = "invalid button name";
            return false;
        }

        if (std::string(action) == "tap")
        {
            append_button_step(MCP_INPUT_MACRO_STEP_PRESS, player, button);
            append_wait_step(1);
            append_button_step(MCP_INPUT_MACRO_STEP_RELEASE, player, button);
        }
        else if (std::string(action) == "press")
            append_button_step(MCP_INPUT_MACRO_STEP_PRESS, player, button);
        else
            append_button_step(MCP_INPUT_MACRO_STEP_RELEASE, player, button);

        return true;
    }

    bool is_valid_button_name(const std::string& button) const
    {
        std::string button_lower = button;
        std::transform(button_lower.begin(), button_lower.end(), button_lower.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });

        return button_lower == "up" || button_lower == "down" ||
            button_lower == "left" || button_lower == "right" ||
            button_lower == "a" || button_lower == "b" ||
            button_lower == "start" || button_lower == "select";
    }

    void append_button_step(McpInputMacroStepType type, int player, const std::string& button)
    {
        McpInputMacroStep step;
        step.type = type;
        step.player = player;
        step.button = button;
        step.frames = 0;
        m_inputMacro.steps.push_back(step);
    }

    void append_wait_step(int frames)
    {
        McpInputMacroStep step;
        step.type = MCP_INPUT_MACRO_STEP_WAIT;
        step.player = 1;
        step.button.clear();
        step.frames = frames;
        m_inputMacro.steps.push_back(step);
    }

    void pump_input_macro(GearboyCore* core)
    {
        if (m_inputMacro.waiting)
        {
            if (emu_frame_counter < m_inputMacro.wait_target_frame)
            {
                continue_input_macro_wait();
                return;
            }

            m_inputMacro.waiting = false;
        }

        while (m_inputMacro.step_index < m_inputMacro.steps.size())
        {
            const McpInputMacroStep& step = m_inputMacro.steps[m_inputMacro.step_index];

            if (step.type == MCP_INPUT_MACRO_STEP_WAIT)
            {
                m_inputMacro.frames_waited += step.frames;
                m_inputMacro.wait_target_frame = emu_frame_counter + (u64)step.frames;
                m_inputMacro.waiting = true;
                m_inputMacro.step_index++;

                continue_input_macro_wait();
                return;
            }

            const char* action = (step.type == MCP_INPUT_MACRO_STEP_PRESS) ? "press" : "release";
            json action_result = m_debugAdapter->ControllerButton(step.player, step.button, action);

            if (action_result.contains("error"))
            {
                finish_input_macro_error(action_result["error"]);
                return;
            }

            handle_controller_side_effects(core, action_result);
            m_inputMacro.step_index++;
        }

        finish_input_macro_success();
    }

    void continue_input_macro_wait()
    {
        if (emu_is_debug_idle())
        {
            u64 frames = m_inputMacro.wait_target_frame - emu_frame_counter;

            if (frames > 1000)
                frames = 1000;

            if (frames > 0)
                m_debugAdapter->StepFrame((int)frames);
        }
        else if (m_inputMacro.restore_pause && emu_is_paused())
            emu_resume();
    }

    void finish_input_macro_success()
    {
        bool restore_pause = m_inputMacro.restore_pause;

        DebugResponse* resp = new DebugResponse();
        resp->requestId = m_inputMacro.request_id;
        resp->isError = false;
        resp->result = {
            {"success", true},
            {"commands", m_inputMacro.command_count},
            {"steps", (int)m_inputMacro.steps.size()},
            {"frames_waited", m_inputMacro.frames_waited}
        };

        reset_input_macro();
        if (restore_pause)
            emu_pause();

        m_responseQueue.Push(resp);
    }

    void finish_input_macro_error(const std::string& error)
    {
        bool restore_pause = m_inputMacro.restore_pause;

        DebugResponse* resp = new DebugResponse();
        resp->requestId = m_inputMacro.request_id;
        resp->isToolError = true;
        resp->errorMessage = error;
        resp->result = {{"error", error}};

        reset_input_macro();
        if (restore_pause)
            emu_pause();

        m_responseQueue.Push(resp);
    }

    void handle_controller_side_effects(GearboyCore* core, json& result)
    {
        if (result.contains("__delayed_release") && result["__delayed_release"] == true)
        {
            DelayedButtonRelease release;
            release.player = result["player"];
            release.button = result["button"];
            release.release_at_cycle = core->GetMasterClockCycles() + 3000000; // around 10 frames
            m_delayedReleases.push_back(release);

            result.erase("__delayed_release");
        }
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
    McpInputMacroState m_inputMacro;
};

#endif /* MCP_MANAGER_H */
