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

#define GUI_DEBUG_IMPORT
#include "gui_debug.h"

#include <fstream>
#include "gearboy.h"
#include "imgui.h"
#include "gui_debug_disassembler.h"
#include "gui_debug_memory.h"
#include "gui_debug_processor.h"
#include "gui_debug_rewind.h"
#include "gui_debug_video.h"
#include "gui_debug_io.h"
#include "gui_debug_psg.h"
#include "gui_debug_link_cable.h"
#include "gui_debug_trace_logger.h"
#include "gui_debug_sgb.h"
#include "emu.h"
#include "config.h"

static const char* GBDEBUG_MAGIC = "GBDEBUG1";
static const int GBDEBUG_MAGIC_LEN = 8;
static const int GBDEBUG_MAX_RECORDS = 0x10000;

static bool read_settings_data(std::istream& stream, void* data, size_t size);
static bool read_settings_bool(std::istream& stream, bool& value);
static bool read_settings_count(std::istream& stream, int& count, size_t record_size);


void gui_debug_init(void)
{
    gui_debug_trace_logger_init();
    gui_debug_disassembler_init();
    gui_debug_memory_init();
    gui_debug_psg_init();
}

void gui_debug_destroy(void)
{
    gui_debug_trace_logger_shutdown();
    gui_debug_disassembler_destroy();
    gui_debug_psg_destroy();
    gui_debug_memory_destroy();
}

void gui_debug_reset(void)
{
    gui_debug_disassembler_reset();
    gui_debug_memory_reset();
    gui_debug_reset_breakpoints();
    gui_debug_reset_symbols();
}

void gui_debug_update(void)
{
    gui_debug_trace_logger_update();
}

void gui_debug_windows(void)
{
    gui_debug_update();

    emu_get_core()->GetAudio()->EnablePSGDebug(config_debug.debug && config_debug.show_psg);

    if (config_debug.debug)
    {
        if (config_debug.show_processor)
            gui_debug_window_processor();
        if (config_debug.show_memory)
            gui_debug_window_memory();
        if (config_debug.show_disassembler)
            gui_debug_window_disassembler();
        if (config_debug.show_call_stack)
            gui_debug_window_call_stack();
        if (config_debug.show_breakpoints)
            gui_debug_window_breakpoints();
        if (config_debug.show_symbols)
            gui_debug_window_symbols();
        if (config_debug.show_io)
            gui_debug_window_io();
        if (config_debug.show_psg)
            gui_debug_window_psg();
        if (config_debug.show_link_cable)
            gui_debug_window_link_cable();
        if (config_debug.show_link_cable_transport)
            gui_debug_window_link_cable_transport();
        if (config_debug.show_video_nametable)
            gui_debug_window_vram_nametable();
        if (config_debug.show_video_tiles)
            gui_debug_window_vram_tiles();
        if (config_debug.show_video_sprites)
            gui_debug_window_vram_sprites();
        if (config_debug.show_video_palettes)
            gui_debug_window_vram_dmg_palettes();
        if (config_debug.show_video_gbc_palettes)
            gui_debug_window_vram_gbc_palettes();
        if (config_debug.show_trace_logger)
            gui_debug_window_trace_logger();
        if (config_debug.show_rewind)
            gui_debug_window_rewind();
        if (emu_get_core()->IsSGB())
        {
            if (config_debug.show_sgb_state)
                gui_debug_window_sgb_state();
            if (config_debug.show_sgb_video)
                gui_debug_window_sgb_video();
            if (config_debug.show_sgb_palettes)
                gui_debug_window_sgb_palettes();
            if (config_debug.show_sgb_system_palettes)
                gui_debug_window_sgb_system_palettes();
            if (config_debug.show_sgb_border_palettes)
                gui_debug_window_sgb_border_palettes();
        }

        gui_debug_memory_watches_window();
        gui_debug_memory_search_window();
        gui_debug_memory_find_bytes_window();
    }
}

void gui_debug_save_settings(const char* file_path)
{
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        Log("Failed to open debug settings file for writing: %s", file_path);
        return;
    }

    file.write(GBDEBUG_MAGIC, GBDEBUG_MAGIC_LEN);

    GearboyCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();

    std::vector<Processor::GB_Breakpoint>* breakpoints = processor->GetBreakpoints();
    int bp_count = (int)breakpoints->size();
    file.write((const char*)&bp_count, sizeof(int));
    for (int i = 0; i < bp_count; i++)
    {
        Processor::GB_Breakpoint& bp = (*breakpoints)[i];
        file.write((const char*)&bp.enabled, sizeof(bool));
        file.write((const char*)&bp.type, sizeof(int));
        file.write((const char*)&bp.address1, sizeof(u16));
        file.write((const char*)&bp.address2, sizeof(u16));
        file.write((const char*)&bp.read, sizeof(bool));
        file.write((const char*)&bp.write, sizeof(bool));
        file.write((const char*)&bp.execute, sizeof(bool));
        file.write((const char*)&bp.range, sizeof(bool));
    }

    file.write((const char*)&emu_debug_irq_breakpoints, sizeof(bool));

    void* bookmarks_ptr = NULL;
    int bookmark_count = gui_debug_get_disassembler_bookmarks(&bookmarks_ptr);
    file.write((const char*)&bookmark_count, sizeof(int));
    if (bookmark_count > 0 && bookmarks_ptr != NULL)
    {
        struct DasmBookmark { u16 address; char name[32]; };
        std::vector<DasmBookmark>* bm_vec = (std::vector<DasmBookmark>*)bookmarks_ptr;
        for (int i = 0; i < bookmark_count; i++)
        {
            file.write((const char*)&(*bm_vec)[i].address, sizeof(u16));
            file.write((*bm_vec)[i].name, 32);
        }
    }

    gui_debug_memory_save_settings(file);

    file.close();

    Log("Debug settings saved to: %s", file_path);
}

void gui_debug_load_settings(const char* file_path)
{
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        Log("Failed to open debug settings file for reading: %s", file_path);
        return;
    }

    char magic[8] = {};
    if (!read_settings_data(file, magic, GBDEBUG_MAGIC_LEN) ||
        memcmp(magic, GBDEBUG_MAGIC, GBDEBUG_MAGIC_LEN) != 0)
    {
        Log("Invalid debug settings file: %s", file_path);
        return;
    }

    GearboyCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();

    Processor::GB_Breakpoint breakpoint = {};
    size_t breakpoint_size = sizeof(breakpoint.enabled) + sizeof(breakpoint.type) +
        sizeof(breakpoint.address1) + sizeof(breakpoint.address2) + sizeof(breakpoint.read) +
        sizeof(breakpoint.write) + sizeof(breakpoint.execute) + sizeof(breakpoint.range);
    int bp_count = 0;
    if (!read_settings_count(file, bp_count, breakpoint_size))
    {
        Log("Invalid debug settings file: %s", file_path);
        return;
    }

    std::vector<Processor::GB_Breakpoint> breakpoints;
    breakpoints.reserve((size_t)bp_count);
    for (int i = 0; i < bp_count; i++)
    {
        Processor::GB_Breakpoint bp = {};
        if (!read_settings_bool(file, bp.enabled) ||
            !read_settings_data(file, &bp.type, sizeof(bp.type)) ||
            !read_settings_data(file, &bp.address1, sizeof(bp.address1)) ||
            !read_settings_data(file, &bp.address2, sizeof(bp.address2)) ||
            !read_settings_bool(file, bp.read) ||
            !read_settings_bool(file, bp.write) ||
            !read_settings_bool(file, bp.execute) ||
            !read_settings_bool(file, bp.range))
        {
            Log("Invalid debug settings file: %s", file_path);
            return;
        }
        breakpoints.push_back(bp);
    }

    bool irq_breakpoints = false;
    if (!read_settings_bool(file, irq_breakpoints))
    {
        Log("Invalid debug settings file: %s", file_path);
        return;
    }

    struct DasmBookmark { u16 address; char name[32]; };
    DasmBookmark bookmark = {};
    size_t bookmark_size = sizeof(bookmark.address) + sizeof(bookmark.name);
    int bookmark_count = 0;
    if (!read_settings_count(file, bookmark_count, bookmark_size))
    {
        Log("Invalid debug settings file: %s", file_path);
        return;
    }

    std::vector<DasmBookmark> bookmarks;
    bookmarks.reserve((size_t)bookmark_count);
    for (int i = 0; i < bookmark_count; i++)
    {
        DasmBookmark item = {};
        if (!read_settings_data(file, &item.address, sizeof(item.address)) ||
            !read_settings_data(file, item.name, sizeof(item.name)))
        {
            Log("Invalid debug settings file: %s", file_path);
            return;
        }
        item.name[sizeof(item.name) - 1] = 0;
        bookmarks.push_back(item);
    }

    if (!gui_debug_memory_load_settings(file))
    {
        Log("Invalid debug settings file: %s", file_path);
        return;
    }

    processor->GetBreakpoints()->swap(breakpoints);
    emu_debug_irq_breakpoints = irq_breakpoints;

    gui_debug_reset_disassembler_bookmarks();
    for (int i = 0; i < bookmark_count; i++)
        gui_debug_add_disassembler_bookmark(bookmarks[i].address, bookmarks[i].name);

    file.close();

    Log("Debug settings loaded from: %s", file_path);
}

static std::string get_auto_debug_settings_path(void)
{
    GearboyCore* core = emu_get_core();
    if (!core || !core->GetCartridge() || strlen(core->GetCartridge()->GetFileName()) == 0)
        return "";

    std::string filename = core->GetCartridge()->GetFileName();
    std::string::size_type dot = filename.find_last_of('.');
    if (dot != std::string::npos)
        filename = filename.substr(0, dot);
    filename += ".gbdebug";

    std::string path = config_root_path;
    path += filename;
    return path;
}

void gui_debug_auto_save_settings(void)
{
    if (!config_debug.auto_debug_settings)
        return;

    std::string path = get_auto_debug_settings_path();
    if (path.empty())
        return;

    gui_debug_save_settings(path.c_str());
}

void gui_debug_auto_load_settings(void)
{
    if (!config_debug.auto_debug_settings)
        return;

    std::string path = get_auto_debug_settings_path();
    if (path.empty())
        return;

    std::ifstream test(path, std::ios::binary);
    if (!test.is_open())
        return;
    test.close();

    gui_debug_load_settings(path.c_str());
}

static bool read_settings_data(std::istream& stream, void* data, size_t size)
{
    stream.read((char*)data, (std::streamsize)size);
    return !stream.fail() && stream.gcount() == (std::streamsize)size;
}

static bool read_settings_bool(std::istream& stream, bool& value)
{
    u8 data[sizeof(bool)] = {};
    bool false_value = false;
    bool true_value = true;

    if (!read_settings_data(stream, data, sizeof(data)))
        return false;
    if (memcmp(data, &false_value, sizeof(data)) == 0)
    {
        value = false;
        return true;
    }
    if (memcmp(data, &true_value, sizeof(data)) == 0)
    {
        value = true;
        return true;
    }

    return false;
}

static bool read_settings_count(std::istream& stream, int& count, size_t record_size)
{
    if (!read_settings_data(stream, &count, sizeof(count)))
        return false;
    if (count < 0 || count > GBDEBUG_MAX_RECORDS || record_size == 0)
        return false;

    std::streampos position = stream.tellg();
    if (position == std::streampos(-1))
        return false;

    stream.seekg(0, std::ios::end);
    std::streampos end = stream.tellg();
    if (end == std::streampos(-1))
        return false;

    stream.seekg(position);
    if (stream.fail() || end < position)
        return false;

    u64 remaining = (u64)(end - position);
    return (u64)count <= remaining / record_size;
}
