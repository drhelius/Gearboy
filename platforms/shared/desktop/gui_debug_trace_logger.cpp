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

#define GUI_DEBUG_TRACE_LOGGER_IMPORT
#include "gui_debug_trace_logger.h"

#include "imgui.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "gui_debug_constants.h"
#include "gui_debug_text.h"
#include "config.h"
#include "emu.h"
#include "gui_debug.h"
#include "utils.h"
#include "trace_logger_formatter.h"
#include "log.h"
#include <cstring>

#define TRACE_DISK_BUFFER_SIZE (1024 * 1024)
#define TRACE_DISK_STAGING_CAPACITY 100000

static bool trace_logger_enabled = false;
static bool trace_logger_follow_latest = true;
static bool trace_logger_scroll_to_bottom = false;
static bool trace_logger_wait_for_scroll_away = false;
static bool trace_logger_choose_output_path = false;
static FILE* trace_logger_disk_file = NULL;
static char trace_logger_disk_path[4096] = {};
static char trace_logger_disk_directory[4096] = {};
static char trace_logger_disk_buffer[TRACE_DISK_BUFFER_SIZE];
static size_t trace_logger_disk_buffer_used = 0;
static u64 trace_logger_disk_entries = 0;
static u64 trace_logger_disk_flushed_total = 0;
static u64 trace_logger_disk_bytes = 0;
static u64 trace_logger_disk_previous_cycle = 0;
static bool trace_logger_disk_previous_cycle_valid = false;
static bool trace_logger_disk_limit_reached = false;
static bool trace_logger_disk_overflow = false;
static Uint64 trace_logger_disk_last_flush = 0;

static const u32 k_trace_logger_capacities[] = {100000, 500000, 1000000, 2000000, 5000000};
static const char* const k_trace_logger_capacity_names[] = {"100K", "500K", "1M", "2M", "5M"};
static const char* const k_trace_logger_capacity_labels[] = {"100K (4 MB)", "500K (20 MB)", "1M (40 MB)", "2M (80 MB)", "5M (200 MB)"};
static const char* const k_trace_logger_disk_size_names[] = {
    "10MB", "50MB", "100MB", "250MB", "500MB", "1GB", "unbounded"
};
static const u64 k_trace_logger_disk_sizes[] = {
    10ULL * 1024ULL * 1024ULL,
    50ULL * 1024ULL * 1024ULL,
    100ULL * 1024ULL * 1024ULL,
    250ULL * 1024ULL * 1024ULL,
    500ULL * 1024ULL * 1024ULL,
    1024ULL * 1024ULL * 1024ULL,
    0
};

static void trace_logger_menu(void);
static void trace_logger_sync_flags(void);
static u32 trace_logger_get_config_flags(void);
static void trace_logger_set_config_flags(u32 flags);
static u32 trace_logger_get_config_event_filter(GB_Trace_Type type);
static void trace_logger_set_config_event_filter(GB_Trace_Type type, u32 filter);
static void trace_logger_menu_event_filter(const char* label, int* filter, u32 mask);
static bool trace_logger_apply_capacity(void);
static bool trace_logger_start_disk(void);
static bool trace_logger_start(u32 flags, bool update_config);
static bool trace_logger_stop(bool show_status);
static bool trace_logger_stop_disk(bool show_status, bool flush_entries);
static bool trace_logger_flush_disk_buffer(bool flush_file);
static bool trace_logger_flush_disk_entries(void);
static void format_entry_text(const GB_Trace_Entry& entry, bool cycles,
    bool previous_cycle_valid, u64 previous_cycle, char* buf, int buf_size);
static void render_entry_colored(const GB_Trace_Entry& entry, u64 index,
    bool previous_cycle_valid, u64 previous_cycle);

void gui_debug_window_trace_logger(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 168), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(544, 362), ImGuiCond_FirstUseEver);

    ImGui::Begin("Trace Logger", &config_debug.show_trace_logger, ImGuiWindowFlags_MenuBar);

    trace_logger_menu();

    TraceLogger* tl = emu_get_core()->GetTraceLogger();

    if (ImGui::Button(trace_logger_enabled ? "Stop" : "Start"))
    {
        if (trace_logger_enabled)
        {
            gui_debug_trace_logger_stop();
        }
        else
        {
            trace_logger_start(trace_logger_get_config_flags(), false);
        }
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(trace_logger_enabled && config_debug.trace_output == gui_TraceOutput_Disk);
    if (ImGui::Button("Clear"))
    {
        gui_debug_trace_logger_clear();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(trace_logger_enabled);
    ImGui::SetNextItemWidth(90.0f);
    int previous_output = config_debug.trace_output;
    if (ImGui::Combo("##trace_output", &config_debug.trace_output, "Memory\0Disk\0\0"))
    {
        if (!trace_logger_apply_capacity())
            config_debug.trace_output = previous_output;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(trace_logger_enabled);
    ImGui::SetNextItemWidth(145.0f);
    if (config_debug.trace_output == gui_TraceOutput_Memory)
    {
        int previous_capacity = config_debug.trace_capacity;
        if (ImGui::Combo("##trace_capacity", &config_debug.trace_capacity, k_trace_logger_capacity_labels, IM_ARRAYSIZE(k_trace_logger_capacity_labels)) && !trace_logger_apply_capacity())
            config_debug.trace_capacity = previous_capacity;
    }
    else
    {
        ImGui::Combo("##trace_disk_size", &config_debug.trace_disk_size, "10 MB\0" "50 MB\0" "100 MB\0" "250 MB\0" "500 MB\0" "1 GB\0" "Unbounded\0\0");
    }
    ImGui::EndDisabled();
    if (config_debug.trace_output == gui_TraceOutput_Memory && ImGui::IsItemHovered())
    {
        double memory_mib = ((double)k_trace_logger_capacities[config_debug.trace_capacity] * sizeof(GB_Trace_Entry)) / (1024.0 * 1024.0);
        ImGui::SetTooltip("Preallocated memory: %.1f MiB (%u bytes per entry).", memory_mib, (u32)sizeof(GB_Trace_Entry));
    }

    if (config_debug.trace_output == gui_TraceOutput_Memory)
    {
        ImGui::SameLine();
        ImGui::Text("Entries: %u / %u", tl->GetCount(), tl->GetCapacity());
    }
    if (config_debug.trace_output == gui_TraceOutput_Disk && trace_logger_disk_path[0] != '\0')
    {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##trace_disk_file", trace_logger_disk_path, sizeof(trace_logger_disk_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
    }

    if (trace_logger_enabled)
        trace_logger_sync_flags();

    u32 count = tl->GetCount();
    ImGui::PushFont(gui_default_font);
    float line_height = ImGui::GetTextLineHeightWithSpacing();
    float content_height = (float)count * line_height;
    ImGui::SetNextWindowContentSize(ImVec2(0.0f, content_height));
    if ((trace_logger_enabled && trace_logger_follow_latest) || trace_logger_scroll_to_bottom)
        ImGui::SetNextWindowScroll(ImVec2(-1.0f, content_height));

    if (ImGui::BeginChild("##logger", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        float scroll_y = ImGui::GetScrollY();
        float scroll_max_y = ImGui::GetScrollMaxY();
        bool at_bottom = scroll_y >= scroll_max_y - 0.5f;
        bool user_scrolling = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
            (ImGui::GetIO().MouseWheel != 0.0f || ImGui::IsMouseDragging(ImGuiMouseButton_Left));
        if (trace_logger_enabled)
        {
            if (trace_logger_scroll_to_bottom)
            {
                trace_logger_follow_latest = true;
                trace_logger_wait_for_scroll_away = false;
            }
            else if (trace_logger_follow_latest && user_scrolling)
            {
                trace_logger_follow_latest = false;
                trace_logger_wait_for_scroll_away = true;
            }
            else if (!trace_logger_follow_latest)
            {
                if (trace_logger_wait_for_scroll_away)
                {
                    if (!at_bottom)
                        trace_logger_wait_for_scroll_away = false;
                }
                else if (at_bottom)
                    trace_logger_follow_latest = true;
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)count, line_height);

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                const GB_Trace_Entry& entry = tl->GetEntry((u32)item);
                u64 entry_number = tl->GetSequence() - (u64)count + (u64)item;
                bool previous_cycle_valid = item > 0;
                u64 previous_cycle = previous_cycle_valid ? tl->GetEntry((u32)item - 1).cycle : 0;
                render_entry_colored(entry, entry_number, previous_cycle_valid, previous_cycle);
            }
        }

        trace_logger_scroll_to_bottom = false;
    }

    ImGui::EndChild();
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();

    if (trace_logger_choose_output_path)
    {
        trace_logger_choose_output_path = false;
        gui_file_dialog_choose_trace_path();
    }
}

void gui_debug_trace_logger_init(void)
{
    strncpy_fit(trace_logger_disk_directory, config_debug.trace_disk_path.c_str(), sizeof(trace_logger_disk_directory));
    if (!trace_logger_apply_capacity())
    {
        config_debug.trace_capacity = 0;
        trace_logger_apply_capacity();
    }
}

void gui_debug_trace_logger_update(void)
{
    if (trace_logger_enabled && config_debug.trace_output == gui_TraceOutput_Disk)
    {
        if (!trace_logger_flush_disk_entries())
        {
            trace_logger_stop_disk(false, false);
            if (trace_logger_disk_overflow)
                gui_set_error_message("Trace disk staging buffer overflow.");
            else
                gui_set_error_message("Error writing trace log to disk.");
        }
        else if (trace_logger_disk_limit_reached)
        {
            trace_logger_stop_disk(false, false);
            gui_set_status_message("Trace recording stopped: maximum file size reached", 4000);
        }
        else
        {
            Uint64 now = SDL_GetTicks();
            if ((now - trace_logger_disk_last_flush) >= 1000)
            {
                if (!trace_logger_flush_disk_buffer(true))
                {
                    trace_logger_stop_disk(false, false);
                    gui_set_error_message("Error flushing trace log to disk.");
                }
                else
                    trace_logger_disk_last_flush = now;
            }
        }
    }
}

void gui_debug_trace_logger_shutdown(void)
{
    if (trace_logger_disk_file && !trace_logger_stop_disk(false, true))
        Error("Error closing trace log file: %s", trace_logger_disk_path);
}

void gui_debug_trace_logger_clear(void)
{
    TraceLogger* tl = emu_get_core()->GetTraceLogger();
    if (trace_logger_enabled && config_debug.trace_output == gui_TraceOutput_Disk)
    {
        if (!trace_logger_flush_disk_entries())
        {
            bool overflow = trace_logger_disk_overflow;
            trace_logger_stop_disk(false, false);
            if (overflow)
                gui_set_error_message("Trace disk staging buffer overflow.");
            else
                gui_set_error_message("Error writing trace log to disk.");
            return;
        }
        tl->Reset();
        trace_logger_disk_flushed_total = 0;
        trace_logger_disk_previous_cycle = 0;
        trace_logger_disk_previous_cycle_valid = false;
    }
    else
        tl->Reset();
}

void gui_debug_trace_logger_reset(void)
{
    if (!trace_logger_stop(false))
        Error("Error stopping trace logger during reset");

    emu_get_core()->GetTraceLogger()->Reset();
    trace_logger_disk_flushed_total = 0;
    trace_logger_disk_previous_cycle = 0;
    trace_logger_disk_previous_cycle_valid = false;
}

void gui_debug_trace_logger_set_output_directory(const char* path)
{
    strncpy_fit(trace_logger_disk_directory, path, sizeof(trace_logger_disk_directory));
    config_debug.trace_disk_path.assign(path);
}

int gui_debug_trace_logger_memory_size_index(const char* size)
{
    if (size)
    {
        for (int i = 0; i < IM_ARRAYSIZE(k_trace_logger_capacity_names); i++)
        {
            if (strcmp(size, k_trace_logger_capacity_names[i]) == 0)
                return i;
        }
    }
    return -1;
}

int gui_debug_trace_logger_disk_size_index(const char* size)
{
    if (size)
    {
        for (int i = 0; i < IM_ARRAYSIZE(k_trace_logger_disk_size_names); i++)
        {
            if (strcmp(size, k_trace_logger_disk_size_names[i]) == 0)
                return i;
        }
    }
    return -1;
}

const char* gui_debug_trace_logger_memory_size_name(int index)
{
    if (index < 0 || index >= IM_ARRAYSIZE(k_trace_logger_capacity_names))
        return k_trace_logger_capacity_names[0];
    return k_trace_logger_capacity_names[index];
}

const char* gui_debug_trace_logger_disk_size_name(int index)
{
    if (index < 0 || index >= IM_ARRAYSIZE(k_trace_logger_disk_size_names))
        return k_trace_logger_disk_size_names[2];
    return k_trace_logger_disk_size_names[index];
}

bool gui_debug_trace_logger_configure(int output, int memory_size, int disk_size, const char* output_path)
{
    if (trace_logger_enabled)
        return false;
    if (output < gui_TraceOutput_Memory || output > gui_TraceOutput_Disk)
        return false;
    if (memory_size < 0 || memory_size >= IM_ARRAYSIZE(k_trace_logger_capacities))
        return false;
    if (disk_size < 0 || disk_size >= IM_ARRAYSIZE(k_trace_logger_disk_sizes))
        return false;

    int previous_output = config_debug.trace_output;
    int previous_memory_size = config_debug.trace_capacity;
    int previous_disk_size = config_debug.trace_disk_size;
    int previous_dir_option = config_debug.trace_disk_dir_option;
    std::string previous_path = config_debug.trace_disk_path;

    config_debug.trace_output = output;
    config_debug.trace_capacity = memory_size;
    config_debug.trace_disk_size = disk_size;
    if (output == gui_TraceOutput_Disk && output_path && output_path[0] != '\0')
    {
        config_debug.trace_disk_dir_option = Directory_Location_Custom;
        gui_debug_trace_logger_set_output_directory(output_path);
    }

    if (!trace_logger_apply_capacity())
    {
        config_debug.trace_output = previous_output;
        config_debug.trace_capacity = previous_memory_size;
        config_debug.trace_disk_size = previous_disk_size;
        config_debug.trace_disk_dir_option = previous_dir_option;
        config_debug.trace_disk_path = previous_path;
        strncpy_fit(trace_logger_disk_directory, previous_path.c_str(), sizeof(trace_logger_disk_directory));
        return false;
    }

    return true;
}

bool gui_debug_trace_logger_start(u32 flags)
{
    return trace_logger_start(flags, true);
}

static bool trace_logger_start(u32 flags, bool update_config)
{
    if (flags == 0)
    {
        flags = TRACE_FLAG_CPU | TRACE_FLAG_CPU_IRQ;
        update_config = true;
    }
    if (update_config)
        trace_logger_set_config_flags(flags);

    if (trace_logger_enabled)
    {
        trace_logger_sync_flags();
        return true;
    }

    if (config_debug.trace_output == gui_TraceOutput_Disk && !trace_logger_start_disk())
        return false;

    trace_logger_enabled = true;
    trace_logger_follow_latest = true;
    trace_logger_scroll_to_bottom = true;
    trace_logger_wait_for_scroll_away = false;
    trace_logger_sync_flags();
    return true;
}

bool gui_debug_trace_logger_stop(void)
{
    return trace_logger_stop(true);
}

static bool trace_logger_stop(bool show_status)
{
    if (!trace_logger_enabled)
        return true;

    trace_logger_scroll_to_bottom = trace_logger_follow_latest;

    if (config_debug.trace_output == gui_TraceOutput_Disk)
        return trace_logger_stop_disk(show_status, true);
    else
    {
        trace_logger_enabled = false;
        emu_get_core()->GetTraceLogger()->SetEnabledFlags(0);
    }
    return true;
}

bool gui_debug_trace_logger_is_enabled(void)
{
    return trace_logger_enabled;
}

const char* gui_debug_trace_logger_get_output_path(void)
{
    return trace_logger_disk_path;
}

void gui_debug_save_log(const char* file_path)
{
    FILE* file = fopen_utf8(file_path, "w");

    if (file != NULL)
    {
        TraceLogger* tl = emu_get_core()->GetTraceLogger();
        u32 count = tl->GetCount();
        u64 oldest = tl->GetSequence() - (u64)count;
        char buf[GB_TRACE_FORMAT_BUFFER_SIZE];

        for (u32 i = 0; i < count; i++)
        {
            const GB_Trace_Entry& entry = tl->GetEntry(i);
            bool previous_cycle_valid = i > 0;
            u64 previous_cycle = previous_cycle_valid ? tl->GetEntry(i - 1).cycle : 0;
            format_entry_text(entry, config_debug.trace_cycles,
                              previous_cycle_valid, previous_cycle, buf, sizeof(buf));
            if (config_debug.trace_counter)
                fprintf(file, "%06llu %s\n", (unsigned long long)(oldest + i), buf);
            else
                fprintf(file, "%s\n", buf);
        }

        fclose(file);
    }
}

static void trace_logger_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Log As...", NULL, false, config_debug.trace_output == gui_TraceOutput_Memory))
        {
            gui_file_dialog_save_log();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Settings"))
    {
        ImGui::MenuItem("Event Counter", "", &config_debug.trace_counter);
        ImGui::MenuItem("Master Clock Cycles", "", &config_debug.trace_cycles);

        if (ImGui::BeginMenu("CPU"))
        {
            ImGui::MenuItem("Bank Number", "", &config_debug.trace_bank);
            ImGui::MenuItem("Registers", "", &config_debug.trace_registers);
            ImGui::MenuItem("Flags", "", &config_debug.trace_flags);
            ImGui::MenuItem("Bytes", "", &config_debug.trace_bytes);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Disk Output"))
        {
            ImGui::BeginDisabled(trace_logger_enabled);
            ImGui::SetNextItemWidth(180.0f);
            ImGui::Combo("##trace_disk_dir", &config_debug.trace_disk_dir_option, "Default Location\0Same as ROM\0Custom Location\0\0");

            switch ((Directory_Location)config_debug.trace_disk_dir_option)
            {
                default:
                case Directory_Location_Default:
                    ImGui::Text("%s", config_root_path);
                    break;
                case Directory_Location_ROM:
                    if (!emu_is_empty())
                        ImGui::Text("%s", emu_get_core()->GetCartridge()->GetFileDirectory());
                    break;
                case Directory_Location_Custom:
                    if (ImGui::MenuItem("Choose..."))
                        trace_logger_choose_output_path = true;
                    ImGui::PushItemWidth(450.0f);
                    if (ImGui::InputText("##trace_disk_path", trace_logger_disk_directory, sizeof(trace_logger_disk_directory), ImGuiInputTextFlags_AutoSelectAll))
                        config_debug.trace_disk_path.assign(trace_logger_disk_directory);
                    ImGui::PopItemWidth();
                    break;
            }
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Filters"))
    {
        if (ImGui::BeginMenu("CPU"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_cpu_enabled);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_cpu_enabled);
            ImGui::MenuItem("Instructions", "", &config_debug.trace_cpu);
            ImGui::MenuItem("IRQs", "", &config_debug.trace_cpu_irq);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("LCD"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_lcd);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_lcd);
            trace_logger_menu_event_filter("Register Writes", &config_debug.trace_lcd_events, TRACE_LCD_FILTER_REGISTERS);
            trace_logger_menu_event_filter("Interrupts", &config_debug.trace_lcd_events, TRACE_LCD_FILTER_INTERRUPTS);
            trace_logger_menu_event_filter("DMA", &config_debug.trace_lcd_events, TRACE_LCD_FILTER_DMA);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Input"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_input);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_input);
            trace_logger_menu_event_filter("Reads", &config_debug.trace_input_events, TRACE_INPUT_FILTER_READS);
            trace_logger_menu_event_filter("Writes", &config_debug.trace_input_events, TRACE_INPUT_FILTER_WRITES);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Timer"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_timer);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_timer);
            trace_logger_menu_event_filter("Interrupts", &config_debug.trace_timer_events, TRACE_TIMER_FILTER_INTERRUPTS);
            trace_logger_menu_event_filter("Register Writes", &config_debug.trace_timer_events, TRACE_TIMER_FILTER_REGISTERS);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("APU"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_apu);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_apu);
            trace_logger_menu_event_filter("Global / Mixer", &config_debug.trace_apu_events, TRACE_APU_FILTER_GLOBAL);
            trace_logger_menu_event_filter("Pulse 1", &config_debug.trace_apu_events, TRACE_APU_FILTER_PULSE1);
            trace_logger_menu_event_filter("Pulse 2", &config_debug.trace_apu_events, TRACE_APU_FILTER_PULSE2);
            trace_logger_menu_event_filter("Wave", &config_debug.trace_apu_events, TRACE_APU_FILTER_WAVE);
            trace_logger_menu_event_filter("Noise", &config_debug.trace_apu_events, TRACE_APU_FILTER_NOISE);
            trace_logger_menu_event_filter("Wave RAM", &config_debug.trace_apu_events, TRACE_APU_FILTER_WAVE_RAM);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Serial"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_serial);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_serial);
            trace_logger_menu_event_filter("Register Writes", &config_debug.trace_serial_events, TRACE_SERIAL_FILTER_REGISTERS);
            trace_logger_menu_event_filter("Transfers", &config_debug.trace_serial_events, TRACE_SERIAL_FILTER_TRANSFERS);
            trace_logger_menu_event_filter("Interrupts", &config_debug.trace_serial_events, TRACE_SERIAL_FILTER_INTERRUPTS);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Mapper"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_mapper);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_mapper);
            trace_logger_menu_event_filter("ROM Mapping", &config_debug.trace_mapper_events, TRACE_MAPPER_FILTER_ROM);
            trace_logger_menu_event_filter("RAM / RTC", &config_debug.trace_mapper_events, TRACE_MAPPER_FILTER_RAM_RTC);
            trace_logger_menu_event_filter("Control", &config_debug.trace_mapper_events, TRACE_MAPPER_FILTER_CONTROL);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

static bool trace_logger_apply_capacity(void)
{
    TraceLogger* tl = emu_get_core()->GetTraceLogger();
    u32 capacity = TRACE_DISK_STAGING_CAPACITY;
    if (config_debug.trace_output == gui_TraceOutput_Memory)
        capacity = k_trace_logger_capacities[config_debug.trace_capacity];

    if (!tl->SetCapacity(capacity))
    {
        gui_set_error_message("Unable to allocate the selected trace logger capacity.");
        return false;
    }
    return true;
}

static bool trace_logger_start_disk(void)
{
    if (!trace_logger_apply_capacity())
        return false;

    const char* directory = config_root_path;
    switch ((Directory_Location)config_debug.trace_disk_dir_option)
    {
        case Directory_Location_ROM:
            if (!emu_is_empty())
                directory = emu_get_core()->GetCartridge()->GetFileDirectory();
            break;
        case Directory_Location_Custom:
            directory = config_debug.trace_disk_path.c_str();
            break;
        default:
            break;
    }

    time_t now = time(0);
    tm local_time;
    char date_time[32] = {};
    if (get_local_time(now, &local_time))
        strftime(date_time, sizeof(date_time), "%Y-%m-%d %H%M%S", &local_time);

    const char* rom_name = "Gearboy";
    if (!emu_is_empty())
        rom_name = emu_get_core()->GetCartridge()->GetFileName();

    bool path_available = false;
    for (int index = 0; index < 1000; index++)
    {
        char filename[1024];
        if (index == 0)
            snprintf(filename, sizeof(filename), "%s - Trace - %s.txt", rom_name, date_time);
        else
            snprintf(filename, sizeof(filename), "%s - Trace - %s (%d).txt", rom_name, date_time, index + 1);

        if (!join_path(directory, filename, trace_logger_disk_path, sizeof(trace_logger_disk_path)))
        {
            gui_set_error_message("Trace log path is too long.");
            return false;
        }
        if (!path_exists(trace_logger_disk_path))
        {
            path_available = true;
            break;
        }
    }

    if (!path_available)
    {
        gui_set_error_message("Unable to create a unique trace log filename.");
        return false;
    }

    trace_logger_disk_file = fopen_utf8(trace_logger_disk_path, "wb");
    if (!trace_logger_disk_file)
    {
        gui_set_error_message("Unable to create the trace log file.");
        trace_logger_disk_path[0] = '\0';
        return false;
    }

    trace_logger_disk_buffer_used = 0;
    trace_logger_disk_entries = 0;
    trace_logger_disk_flushed_total = 0;
    trace_logger_disk_bytes = 0;
    trace_logger_disk_previous_cycle = 0;
    trace_logger_disk_previous_cycle_valid = false;
    trace_logger_disk_limit_reached = false;
    trace_logger_disk_overflow = false;
    trace_logger_disk_last_flush = SDL_GetTicks();
    emu_get_core()->GetTraceLogger()->Reset();
    gui_set_status_message("Trace recording started", 3000);
    return true;
}

static bool trace_logger_stop_disk(bool show_status, bool flush_entries)
{
    bool success = trace_logger_disk_file != NULL;

    if (trace_logger_disk_file)
    {
        if (flush_entries && !trace_logger_flush_disk_entries())
            success = false;
        if (!trace_logger_flush_disk_buffer(true))
            success = false;
        if (fclose(trace_logger_disk_file) != 0)
            success = false;
        trace_logger_disk_file = NULL;
    }

    trace_logger_enabled = false;
    emu_get_core()->GetTraceLogger()->SetEnabledFlags(0);
    if (show_status)
    {
        if (success)
            gui_set_status_message("Trace recording stopped", 3000);
        else if (trace_logger_disk_overflow)
            gui_set_error_message("Trace recording stopped: staging buffer overflow.");
        else
            gui_set_error_message("Trace recording stopped with a disk write error.");
    }
    return success;
}

static bool trace_logger_flush_disk_buffer(bool flush_file)
{
    if (!trace_logger_disk_file)
        return false;

    if (trace_logger_disk_buffer_used > 0)
    {
        size_t written = fwrite(trace_logger_disk_buffer, 1, trace_logger_disk_buffer_used, trace_logger_disk_file);
        if (written > 0)
        {
            trace_logger_disk_buffer_used -= written;
            if (trace_logger_disk_buffer_used > 0)
            {
                memmove(trace_logger_disk_buffer, trace_logger_disk_buffer + written,
                        trace_logger_disk_buffer_used);
            }
        }
        if (trace_logger_disk_buffer_used > 0)
            return false;
    }

    return !flush_file || fflush(trace_logger_disk_file) == 0;
}

static bool trace_logger_flush_disk_entries(void)
{
    if (!trace_logger_disk_file)
        return false;
    if (trace_logger_disk_limit_reached)
        return true;

    TraceLogger* tl = emu_get_core()->GetTraceLogger();
    u32 count = tl->GetCount();
    u64 total = tl->GetTotalLogged();
    u64 oldest = total - (u64)count;
    if (trace_logger_disk_flushed_total < oldest)
    {
        trace_logger_disk_overflow = true;
        return false;
    }
    u32 first = (u32)(trace_logger_disk_flushed_total - oldest);
    char entry_text[GB_TRACE_FORMAT_BUFFER_SIZE];
    char line[GB_TRACE_FORMAT_BUFFER_SIZE + 64];

    for (u32 i = first; i < count; i++)
    {
        const GB_Trace_Entry& entry = tl->GetEntry(i);
        format_entry_text(entry, config_debug.trace_cycles,
                          trace_logger_disk_previous_cycle_valid,
                          trace_logger_disk_previous_cycle, entry_text, sizeof(entry_text));
        int length;
        if (config_debug.trace_counter)
            length = snprintf(line, sizeof(line), "%06llu %s\n", (unsigned long long)trace_logger_disk_entries, entry_text);
        else
            length = snprintf(line, sizeof(line), "%s\n", entry_text);
        if (length < 0)
            return false;

        size_t line_size = MIN((size_t)length, sizeof(line) - 1);
        u64 max_size = k_trace_logger_disk_sizes[config_debug.trace_disk_size];
        if (max_size > 0 && trace_logger_disk_bytes + (u64)line_size > max_size)
        {
            trace_logger_disk_limit_reached = true;
            break;
        }
        if (trace_logger_disk_buffer_used + line_size > sizeof(trace_logger_disk_buffer) && !trace_logger_flush_disk_buffer(false))
            return false;
        memcpy(trace_logger_disk_buffer + trace_logger_disk_buffer_used, line, line_size);
        trace_logger_disk_buffer_used += line_size;
        trace_logger_disk_entries++;
        trace_logger_disk_bytes += (u64)line_size;
        trace_logger_disk_previous_cycle = entry.cycle;
        trace_logger_disk_previous_cycle_valid = true;
    }

    trace_logger_disk_flushed_total = total;
    return true;
}

static void trace_logger_sync_flags(void)
{
    TraceLogger* tl = emu_get_core()->GetTraceLogger();
    tl->SetEnabledFlags(trace_logger_get_config_flags());
    for (int i = 0; i < TRACE_TYPE_COUNT; i++)
        tl->SetEventFilter((GB_Trace_Type)i, trace_logger_get_config_event_filter((GB_Trace_Type)i));
}

static u32 trace_logger_get_config_flags(void)
{
    u32 flags = 0;
    if (config_debug.trace_cpu_enabled && config_debug.trace_cpu) flags |= TRACE_FLAG_CPU;
    if (config_debug.trace_cpu_enabled && config_debug.trace_cpu_irq) flags |= TRACE_FLAG_CPU_IRQ;
    if (config_debug.trace_lcd) flags |= TRACE_FLAG_LCD;
    if (config_debug.trace_input) flags |= TRACE_FLAG_INPUT;
    if (config_debug.trace_timer) flags |= TRACE_FLAG_TIMER;
    if (config_debug.trace_apu) flags |= TRACE_FLAG_APU;
    if (config_debug.trace_serial) flags |= TRACE_FLAG_SERIAL;
    if (config_debug.trace_mapper) flags |= TRACE_FLAG_MAPPER;
    return flags;
}

static void trace_logger_set_config_flags(u32 flags)
{
    config_debug.trace_cpu_enabled = (flags & (TRACE_FLAG_CPU | TRACE_FLAG_CPU_IRQ)) != 0;
    config_debug.trace_cpu = (flags & TRACE_FLAG_CPU) != 0;
    config_debug.trace_cpu_irq = (flags & TRACE_FLAG_CPU_IRQ) != 0;
    config_debug.trace_lcd = (flags & TRACE_FLAG_LCD) != 0;
    config_debug.trace_input = (flags & TRACE_FLAG_INPUT) != 0;
    config_debug.trace_timer = (flags & TRACE_FLAG_TIMER) != 0;
    config_debug.trace_apu = (flags & TRACE_FLAG_APU) != 0;
    config_debug.trace_serial = (flags & TRACE_FLAG_SERIAL) != 0;
    config_debug.trace_mapper = (flags & TRACE_FLAG_MAPPER) != 0;
}

static u32 trace_logger_get_config_event_filter(GB_Trace_Type type)
{
    switch (type)
    {
        case TRACE_LCD: return (u32)config_debug.trace_lcd_events;
        case TRACE_INPUT: return (u32)config_debug.trace_input_events;
        case TRACE_TIMER: return (u32)config_debug.trace_timer_events;
        case TRACE_APU: return (u32)config_debug.trace_apu_events;
        case TRACE_SERIAL: return (u32)config_debug.trace_serial_events;
        case TRACE_MAPPER: return (u32)config_debug.trace_mapper_events;
        default: return 0xFFFFFFFFU;
    }
}

static void trace_logger_set_config_event_filter(GB_Trace_Type type, u32 filter)
{
    switch (type)
    {
        case TRACE_LCD: config_debug.trace_lcd_events = (int)filter; break;
        case TRACE_INPUT: config_debug.trace_input_events = (int)filter; break;
        case TRACE_TIMER: config_debug.trace_timer_events = (int)filter; break;
        case TRACE_APU: config_debug.trace_apu_events = (int)filter; break;
        case TRACE_SERIAL: config_debug.trace_serial_events = (int)filter; break;
        case TRACE_MAPPER: config_debug.trace_mapper_events = (int)filter; break;
        default: break;
    }
}

static void trace_logger_menu_event_filter(const char* label, int* filter, u32 mask)
{
    bool enabled = ((u32)*filter & mask) != 0;
    if (ImGui::MenuItem(label, "", &enabled))
    {
        if (enabled)
            *filter |= (int)mask;
        else
            *filter &= ~(int)mask;
    }
}

void gui_debug_trace_logger_set_event_filters(const u32* filters)
{
    if (!filters)
        return;

    for (int i = 0; i < TRACE_TYPE_COUNT; i++)
        trace_logger_set_config_event_filter((GB_Trace_Type)i, filters[i]);
}

static void format_entry_text(const GB_Trace_Entry& entry, bool cycles,
    bool previous_cycle_valid, u64 previous_cycle, char* buf, int buf_size)
{
    GB_Trace_Format_Options options;
    options.bank = config_debug.trace_bank;
    options.registers = config_debug.trace_registers;
    options.flags = config_debug.trace_flags;
    options.bytes = config_debug.trace_bytes;
    options.cycles = cycles;
    options.previous_cycle_valid = previous_cycle_valid;
    options.previous_cycle = previous_cycle;
    trace_log_format_entry(emu_get_core()->GetMemory(), entry, options, buf, buf_size);
}

static void render_cpu_entry_colored(const GB_Trace_Entry& entry, int prefix_length)
{
    Memory* memory = emu_get_core()->GetMemory();
    GS_Disassembler_Record* record = trace_log_get_cpu_record(memory, entry);

    if (config_debug.trace_bank)
    {
        ImGui::TextColored(violet, "%03X:", entry.cpu.bank);
        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(cyan, "%04X", entry.cpu.pc);

    u8 a = (entry.cpu.af >> 8) & 0xFF;
    u8 f = entry.cpu.af & 0xFF;

    if (config_debug.trace_registers)
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  A:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%02X", a);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  BC:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.bc);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  DE:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.de);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  HL:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.hl);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  SP:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.sp);
    }

    if (config_debug.trace_flags)
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(yellow, " %c%c%c%c",
                 (f & FLAG_ZERO) ? 'Z' : 'z',
                 (f & FLAG_SUB) ? 'N' : 'n',
                 (f & FLAG_HALF) ? 'H' : 'h',
                 (f & FLAG_CARRY) ? 'C' : 'c');
    }

    if (IsValidPointer(record))
    {
        std::string instr = record->name;
        size_t pos;
        pos = instr.find("{n}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_white);
        pos = instr.find("{o}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_brown);
        pos = instr.find("{e}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_blue);

        ImGui::SameLine(0, 0);
        TextColoredEx("  %s%s", c_white.c_str(), instr.c_str());

    }
    else
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(gray, "  ???");
    }

    if (config_debug.trace_bytes)
    {
        char bytes[16];
        trace_log_format_cpu_bytes(entry, bytes, sizeof(bytes));
        float char_width = ImGui::CalcTextSize("A").x;
        float bytes_column = char_width * 37;
        if (config_debug.trace_registers) bytes_column += char_width * 34;
        if (config_debug.trace_flags)     bytes_column += char_width * 6;
        if (config_debug.trace_bank)      bytes_column += char_width * 4;
        bytes_column += char_width * prefix_length;
        ImGui::SameLine(bytes_column);
        ImGui::TextColored(gray, "%s%s", bytes, entry.cpu.halt_bug ? "[HALT bug]" : "");
    }
}

static void render_entry_colored(const GB_Trace_Entry& entry, u64 index,
    bool previous_cycle_valid, u64 previous_cycle)
{
    char buf[GB_TRACE_FORMAT_BUFFER_SIZE];
    int prefix_length = 0;

    if (config_debug.trace_counter)
    {
        char counter[32];
        snprintf(counter, sizeof(counter), "%06llu ", (unsigned long long)index);
        prefix_length += (int)strlen(counter);
        ImGui::TextColored(gray, "%s", counter);
        ImGui::SameLine(0, 0);
    }

    if (config_debug.trace_cycles)
    {
        char cycles[64];
        trace_log_format_cycle_prefix(entry, previous_cycle_valid, previous_cycle,
                                      cycles, sizeof(cycles));
        prefix_length += (int)strlen(cycles);
        ImGui::TextColored(gray, "%s", cycles);
        ImGui::SameLine(0, 0);
    }

    switch (entry.type)
    {
        case TRACE_CPU:
            render_cpu_entry_colored(entry, prefix_length);
            break;
        case TRACE_CPU_IRQ:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(red, "%s", buf);
            break;
        case TRACE_LCD:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(green, "%s", buf);
            break;
        case TRACE_INPUT:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(yellow, "%s", buf);
            break;
        case TRACE_TIMER:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(orange, "%s", buf);
            break;
        case TRACE_APU:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(blue, "%s", buf);
            break;
        case TRACE_SERIAL:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(cyan, "%s", buf);
            break;
        case TRACE_MAPPER:
            format_entry_text(entry, false, false, 0, buf, sizeof(buf));
            ImGui::TextColored(magenta, "%s", buf);
            break;
        default:
            break;
    }
}
