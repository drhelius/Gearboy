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

#include "mcp_tool_registry.h"
#include <cctype>

struct McpToolCategory
{
    const char* name;
    const char* title;
    const char* description;
};

struct McpToolCategoryTools
{
    const char* category;
    const char* const* tools;
    size_t count;
};

#define MCP_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

static const McpToolCategory kMcpToolCategories[] =
{
    {"execution", "Execution Control", "Pause, resume, step, frame-step, reset, run-to-address, and fast-forward emulator execution."},
    {"breakpoints", "Breakpoints", "Set, clear, toggle, and list CPU execution/read/write/range breakpoints and interrupt breakpoints."},
    {"memory", "Memory", "List memory areas, read/write bytes, select ranges, fill selections, search memory, and manage watches/bookmarks."},
    {"cpu", "CPU", "Inspect SM83 registers, flags, interrupts, PC, stack, and write CPU register values."},
    {"disassembly", "Disassembly", "Read executed-code disassembly, run to addresses, inspect call stacks, and manage disassembly bookmarks."},
    {"symbols", "Symbols", "Add, remove, load, and list debug symbols or labels used by disassembly and debugger views."},
    {"hardware_lcd", "LCD Hardware", "Inspect Game Boy LCD registers, display timing, status, palettes, and video state."},
    {"hardware_audio", "Audio Hardware", "Inspect Game Boy APU state, channels, mixer, volume, and sound registers."},
    {"hardware_sgb", "Super Game Boy", "Inspect Super Game Boy command, border, mask, multiplayer, palette, and transfer state."},
    {"media", "Media", "Load ROMs, list recent media, load symbols, and inspect loaded cartridge/media information."},
    {"capture", "Capture", "Capture current screenshots and Game Boy sprite images or sprite metadata."},
    {"state", "Save States", "List save slots, select a slot, save emulator state, and load emulator state."},
    {"rewind", "Rewind", "Inspect rewind buffer status and seek to rewind snapshots for time-travel debugging."},
    {"input", "Input", "Press, release, tap, or macro Game Boy joypad buttons."},
    {"trace", "Trace", "Read trace log entries and configure CPU, interrupt, video, audio, memory, and debug-message tracing."},
    {"tools", "Other Tools", "Additional emulator/debugger tools that do not fit another category."}
};

static const char* const kMcpExecutionTools[] =
{
    "debug_pause", "debug_continue", "debug_step_into", "debug_step_over", "debug_step_out",
    "debug_step_frame", "debug_run_to_cursor", "debug_reset", "debug_get_status",
    "set_fast_forward_speed", "toggle_fast_forward"
};

static const char* const kMcpBreakpointTools[] =
{
    "set_breakpoint", "set_breakpoint_range", "remove_breakpoint", "list_breakpoints",
    "toggle_irq_breakpoints"
};

static const char* const kMcpMemoryTools[] =
{
    "list_memory_areas", "read_memory", "write_memory", "select_memory_range",
    "set_memory_selection_value", "get_memory_selection", "add_memory_bookmark",
    "remove_memory_bookmark", "list_memory_bookmarks", "add_memory_watch", "remove_memory_watch",
    "list_memory_watches", "memory_search_capture", "memory_search", "memory_find_bytes"
};

static const char* const kMcpCpuTools[] =
{
    "get_cpu_status", "write_cpu_register"
};

static const char* const kMcpDisassemblyTools[] =
{
    "get_disassembly", "add_disassembler_bookmark", "remove_disassembler_bookmark",
    "list_disassembler_bookmarks", "get_call_stack"
};

static const char* const kMcpSymbolTools[] =
{
    "add_symbol", "remove_symbol", "list_symbols", "load_symbols"
};

static const char* const kMcpLcdTools[] =
{
    "get_lcd_registers", "get_lcd_status"
};

static const char* const kMcpAudioTools[] =
{
    "get_apu_status"
};

static const char* const kMcpSgbTools[] =
{
    "get_sgb_status"
};

static const char* const kMcpMediaTools[] =
{
    "load_media", "get_media_info", "list_recent_media"
};

static const char* const kMcpCaptureTools[] =
{
    "get_screenshot", "list_sprites", "get_sprite_image"
};

static const char* const kMcpStateTools[] =
{
    "list_save_state_slots", "select_save_state_slot", "save_state", "load_state"
};

static const char* const kMcpRewindTools[] =
{
    "get_rewind_status", "rewind_seek"
};

static const char* const kMcpInputTools[] =
{
    "controller_button", "controller_macro"
};

static const char* const kMcpTraceTools[] =
{
    "get_trace_log", "set_trace_log"
};

static const McpToolCategoryTools kMcpToolCategoryTools[] =
{
    {"execution", kMcpExecutionTools, MCP_ARRAY_COUNT(kMcpExecutionTools)},
    {"breakpoints", kMcpBreakpointTools, MCP_ARRAY_COUNT(kMcpBreakpointTools)},
    {"memory", kMcpMemoryTools, MCP_ARRAY_COUNT(kMcpMemoryTools)},
    {"cpu", kMcpCpuTools, MCP_ARRAY_COUNT(kMcpCpuTools)},
    {"disassembly", kMcpDisassemblyTools, MCP_ARRAY_COUNT(kMcpDisassemblyTools)},
    {"symbols", kMcpSymbolTools, MCP_ARRAY_COUNT(kMcpSymbolTools)},
    {"hardware_lcd", kMcpLcdTools, MCP_ARRAY_COUNT(kMcpLcdTools)},
    {"hardware_audio", kMcpAudioTools, MCP_ARRAY_COUNT(kMcpAudioTools)},
    {"hardware_sgb", kMcpSgbTools, MCP_ARRAY_COUNT(kMcpSgbTools)},
    {"media", kMcpMediaTools, MCP_ARRAY_COUNT(kMcpMediaTools)},
    {"capture", kMcpCaptureTools, MCP_ARRAY_COUNT(kMcpCaptureTools)},
    {"state", kMcpStateTools, MCP_ARRAY_COUNT(kMcpStateTools)},
    {"rewind", kMcpRewindTools, MCP_ARRAY_COUNT(kMcpRewindTools)},
    {"input", kMcpInputTools, MCP_ARRAY_COUNT(kMcpInputTools)},
    {"trace", kMcpTraceTools, MCP_ARRAY_COUNT(kMcpTraceTools)}
};

const size_t kMcpSearchToolLimit = 20;

McpToolRegistry::McpToolRegistry()
{
    m_tools = json::array();
}

void McpToolRegistry::SetTools(const json& tools)
{
    m_tools = json::array();

    if (!tools.is_array())
        return;

    for (json::const_iterator it = tools.begin(); it != tools.end(); ++it)
    {
        if (!it->is_object() || !it->contains("name") || !(*it)["name"].is_string())
            continue;

        if (IsRouterToolName((*it)["name"].get<std::string>()))
            continue;

        m_tools.push_back(*it);
    }
}

bool McpToolRegistry::IsEmpty() const
{
    return !m_tools.is_array() || m_tools.empty();
}

bool McpToolRegistry::HasTool(const std::string& tool_name) const
{
    return FindTool(tool_name) != NULL;
}

bool McpToolRegistry::HasCategory(const std::string& category) const
{
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);

    for (size_t i = 0; i < category_count; i++)
    {
        if ((category == kMcpToolCategories[i].name) && HasToolInCategory(category, false))
            return true;
    }

    return false;
}

json McpToolRegistry::GetStats() const
{
    json categories = json::array();
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);
    int routed_count = 0;
    int direct_count = 0;

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();

        if (IsDirectToolName(name))
            direct_count++;
        else
            routed_count++;
    }

    for (size_t i = 0; i < category_count; i++)
    {
        int tool_count = CountToolsInCategory(kMcpToolCategories[i].name, false);

        if (tool_count == 0)
            continue;

        categories.push_back({
            {"name", kMcpToolCategories[i].name},
            {"tool_count", tool_count}
        });
    }

    return {
        {"total_categories", categories.size()},
        {"total_routed_tools", routed_count},
        {"total_direct_tools", direct_count},
        {"total_tools", routed_count + direct_count},
        {"categories", categories}
    };
}

json McpToolRegistry::GetCategories() const
{
    json categories = json::array();
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);

    for (size_t i = 0; i < category_count; i++)
    {
        if (!HasRoutedToolInCategory(kMcpToolCategories[i].name))
            continue;

        categories.push_back({
            {"name", kMcpToolCategories[i].name},
            {"title", kMcpToolCategories[i].title},
            {"description", kMcpToolCategories[i].description},
            {"tool_count", CountToolsInCategory(kMcpToolCategories[i].name, false)}
        });
    }

    return categories;
}

json McpToolRegistry::GetCategoryNames() const
{
    json categories = json::array();
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);

    for (size_t i = 0; i < category_count; i++)
    {
        if (HasRoutedToolInCategory(kMcpToolCategories[i].name))
            categories.push_back(kMcpToolCategories[i].name);
    }

    return categories;
}

json McpToolRegistry::GetDirectTools() const
{
    json tools = json::array();

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();
        if (IsDirectToolName(name))
            tools.push_back(*it);
    }

    return tools;
}

json McpToolRegistry::GetToolsInCategory(const std::string& category) const
{
    json tools = json::array();

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();

        if (IsDirectToolName(name))
            continue;

        if (ToolCategoryForName(name) == category)
            tools.push_back(ToolToSummaryJson(*it));
    }

    return tools;
}

json McpToolRegistry::GetToolInfo(const std::string& tool_name) const
{
    const json* tool = FindTool(tool_name);

    if (tool == NULL)
        return json::object();

    return ToolToInfoJson(*tool);
}

std::string McpToolRegistry::GetCategoryTitle(const std::string& category) const
{
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);

    for (size_t i = 0; i < category_count; i++)
    {
        if (category == kMcpToolCategories[i].name)
            return kMcpToolCategories[i].title;
    }

    return "";
}

std::string McpToolRegistry::GetCategoryDescription(const std::string& category) const
{
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategories);

    for (size_t i = 0; i < category_count; i++)
    {
        if (category == kMcpToolCategories[i].name)
            return kMcpToolCategories[i].description;
    }

    return "";
}

int McpToolRegistry::GetCategoryToolCount(const std::string& category) const
{
    return CountToolsInCategory(category, false);
}

json McpToolRegistry::SearchTools(const std::string& query) const
{
    json tools = json::array();
    std::string query_lower = ToLower(query);

    if (query_lower.empty())
        return tools;

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();

        std::string haystack = name + " ";
        haystack += it->value("title", "");
        haystack += " ";
        haystack += it->value("description", "");
        haystack += " ";
        haystack += ToolCategoryForName(name);
        haystack += " ";
        haystack += AliasesForTool(name);

        if (ToLower(haystack).find(query_lower) != std::string::npos)
        {
            tools.push_back(ToolToSearchJson(*it));

            if (tools.size() >= kMcpSearchToolLimit)
                return tools;
        }
    }

    return tools;
}

bool McpToolRegistry::IsRouterTool(const std::string& tool_name) const
{
    return IsRouterToolName(tool_name);
}

bool McpToolRegistry::IsRouterTool(const std::string& tool_name, const std::string& router_tool_name) const
{
    return NormalizeToolName(tool_name) == router_tool_name;
}

size_t McpToolRegistry::GetSearchToolLimit() const
{
    return kMcpSearchToolLimit;
}

bool McpToolRegistry::IsRouterToolName(const std::string& tool_name) const
{
    std::string name = NormalizeToolName(tool_name);

    return (name == "list_tool_categories") ||
           (name == "get_category_tools") ||
           (name == "get_tool_info") ||
           (name == "search_tools") ||
           (name == "execute_tool");
}

std::string McpToolRegistry::NormalizeToolName(std::string tool_name) const
{
    size_t pos = 0;
    while ((pos = tool_name.find('.', pos)) != std::string::npos)
    {
        tool_name[pos] = '_';
        pos++;
    }

    return tool_name;
}

std::string McpToolRegistry::ToLower(const std::string& text) const
{
    std::string result = text;

    for (size_t i = 0; i < result.size(); i++)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));

    return result;
}

bool McpToolRegistry::StringContains(const std::string& text, const std::string& needle) const
{
    return text.find(needle) != std::string::npos;
}

bool McpToolRegistry::ToolNameInList(const std::string& name, const char* const* tools, size_t count) const
{
    for (size_t i = 0; i < count; i++)
    {
        if (name == tools[i])
            return true;
    }

    return false;
}

bool McpToolRegistry::IsDirectToolName(const std::string& tool_name) const
{
    std::string name = NormalizeToolName(tool_name);

    return (name == "load_media") ||
           (name == "get_media_info") ||
           (name == "debug_pause") ||
           (name == "debug_continue") ||
           (name == "debug_step_into") ||
           (name == "get_6502_status") ||
           (name == "get_cpu_status") ||
           (name == "get_z80_status") ||
           (name == "get_huc6280_status") ||
           (name == "read_memory") ||
           (name == "write_memory") ||
           (name == "get_disassembly") ||
           (name == "set_breakpoint") ||
           (name == "get_screenshot") ||
           (name == "controller_button");
}

std::string McpToolRegistry::ToolCategoryForName(const std::string& tool_name) const
{
    std::string name = NormalizeToolName(tool_name);
    const size_t category_count = MCP_ARRAY_COUNT(kMcpToolCategoryTools);

    for (size_t i = 0; i < category_count; i++)
    {
        if (ToolNameInList(name, kMcpToolCategoryTools[i].tools, kMcpToolCategoryTools[i].count))
            return kMcpToolCategoryTools[i].category;
    }

    return "tools";
}

std::string McpToolRegistry::AliasesForTool(const std::string& tool_name) const
{
    std::string name = NormalizeToolName(tool_name);
    std::string aliases = ToolCategoryForName(name);

    if (StringContains(name, "mikey"))
        aliases += " timers timer irq interrupt hblank vblank audio uart comlynx";
    if (StringContains(name, "suzy") || StringContains(name, "sprite"))
        aliases += " sprites sprite blitter math collision scb";
    if (StringContains(name, "vdp") || StringContains(name, "lcd") ||
        StringContains(name, "huc62"))
        aliases += " video display scanline hblank vblank palette tiles background";
    if (StringContains(name, "apu") || StringContains(name, "psg") ||
        StringContains(name, "audio") || StringContains(name, "ym2413") ||
        StringContains(name, "ay8910"))
        aliases += " sound audio channel tone noise volume";
    if (StringContains(name, "breakpoint"))
        aliases += " watchpoint stop read write execute irq interrupt";
    if (StringContains(name, "memory"))
        aliases += " ram rom vram bytes search watch bookmark selection";
    if (StringContains(name, "symbol"))
        aliases += " label labels names debug symbols";
    if (StringContains(name, "trace"))
        aliases += " log logger events cpu irq debug output";
    if (StringContains(name, "controller"))
        aliases += " input joypad gamepad button macro tap press release";
    if (StringContains(name, "state") || StringContains(name, "rewind"))
        aliases += " save savestate slot snapshot time travel history";
    if (StringContains(name, "cart") || StringContains(name, "eeprom"))
        aliases += " cartridge rom mapper bank save nonvolatile";
    if (StringContains(name, "cdrom") || StringContains(name, "adpcm"))
        aliases += " cd disc track audio pcm";

    return aliases;
}

const json* McpToolRegistry::FindTool(const std::string& tool_name) const
{
    std::string normalized_name = NormalizeToolName(tool_name);

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();
        if (NormalizeToolName(name) == normalized_name)
            return &(*it);
    }

    return NULL;
}

bool McpToolRegistry::HasToolInCategory(const std::string& category, bool include_direct) const
{
    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();

        if (!include_direct && IsDirectToolName(name))
            continue;

        if (ToolCategoryForName(name) == category)
            return true;
    }

    return false;
}

bool McpToolRegistry::HasRoutedToolInCategory(const std::string& category) const
{
    return HasToolInCategory(category, false);
}

int McpToolRegistry::CountToolsInCategory(const std::string& category, bool include_direct) const
{
    int count = 0;

    for (json::const_iterator it = m_tools.begin(); it != m_tools.end(); ++it)
    {
        std::string name = (*it)["name"].get<std::string>();

        if (!include_direct && IsDirectToolName(name))
            continue;

        if (ToolCategoryForName(name) == category)
            count++;
    }

    return count;
}

json McpToolRegistry::ToolToSummaryJson(const json& tool) const
{
    json result;
    std::string name = tool.value("name", "");

    result["name"] = name;
    result["description"] = tool.value("description", "");

    return result;
}

json McpToolRegistry::ToolToSearchJson(const json& tool) const
{
    json result;
    std::string name = tool.value("name", "");

    result["category"] = ToolCategoryForName(name);
    result["tool"] = name;
    result["description"] = tool.value("description", "");

    if (IsDirectToolName(name))
        result["category"] = "direct";

    return result;
}

json McpToolRegistry::ToolToInfoJson(const json& tool) const
{
    json result;
    std::string name = tool.value("name", "");

    result["name"] = name;
    result["title"] = tool.value("title", name);
    result["description"] = tool.value("description", "");
    result["category"] = ToolCategoryForName(name);
    result["direct"] = IsDirectToolName(name);

    if (IsDirectToolName(name))
        result["category"] = "direct";

    if (tool.contains("inputSchema"))
        result["inputSchema"] = tool["inputSchema"];
    else
        result["inputSchema"] = json::object();

    return result;
}
