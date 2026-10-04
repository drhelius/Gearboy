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

#define GUI_DEBUG_PROFILER_IMPORT
#include "gui_debug_profiler.h"

#include <algorithm>
#include <vector>
#include <ctype.h>
#include "imgui.h"
#include "gearboy.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "gui_debug_disassembler.h"
#include "config.h"
#include "emu.h"

#define PROFILER_REFRESH_SECONDS 0.25

enum ProfilerColumn
{
    ProfilerColumn_Function = 0,
    ProfilerColumn_Address,
    ProfilerColumn_Bank,
    ProfilerColumn_Calls,
    ProfilerColumn_CallsPerFrame,
    ProfilerColumn_Inclusive,
    ProfilerColumn_InclusivePercent,
    ProfilerColumn_Exclusive,
    ProfilerColumn_ExclusivePercent,
    ProfilerColumn_Average,
    ProfilerColumn_Min,
    ProfilerColumn_Max,
    ProfilerColumn_Count
};

static const char* const k_profiler_column_tooltips[ProfilerColumn_Count] = {
    "Symbol name (green: manual, yellow: automatic)",
    "Function entry address",
    "ROM bank of the function entry",
    "Times the function was called",
    "Average calls per frame",
    "Cycles in the function and everything it calls, excluding interrupts",
    "Inclusive cycles as a percentage of all profiled cycles",
    "Cycles in the function's own code only",
    "Exclusive cycles as a percentage of all profiled cycles",
    "Average inclusive cycles per completed call",
    "Fewest inclusive cycles in a completed call",
    "Most inclusive cycles in a completed call"
};

struct ProfilerRow
{
    u16 index;
    bool has_symbol;
    bool is_manual;
    char name[64];
};

static bool profiler_visible = false;
static bool profiler_paused = false;
static bool profiler_dirty = true;
static double profiler_refresh_time = 0.0;
static u32 profiler_function_count = 0;
static char profiler_filter[64] = "";
static int profiler_sort_column = ProfilerColumn_Inclusive;
static bool profiler_sort_ascending = false;
static std::vector<ProfilerRow> profiler_rows;
static const GB_Profiler_Function* profiler_sort_functions = NULL;

static void draw_profiler(Profiler* profiler);
static void build_rows(const GB_Profiler_Function* functions, u32 count);
static bool row_matches_filter(const ProfilerRow& row, const GB_Profiler_Function& function, const char* filter);
static bool is_pseudo_function(const GB_Profiler_Function& function);
static u32 get_completed_calls(const GB_Profiler_Function& function);
static u64 get_sort_value(const GB_Profiler_Function& function, int column);
static bool row_sort_compare(const ProfilerRow& a, const ProfilerRow& b);
static void draw_right_aligned(const ImVec4& color, const char* text);
static void draw_number(u64 value);
static void draw_percent(u64 value, u64 total);
static void draw_calls_per_frame(u32 calls, u64 total);
static void draw_empty(void);

void gui_debug_window_profiler(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(947, 383), ImGuiCond_FirstUseEver);

    profiler_visible = ImGui::Begin("Profiler", &config_debug.show_profiler);

    if (profiler_visible)
        draw_profiler(emu_get_core()->GetProfiler());

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_profiler_update(void)
{
    if (!config_debug.debug || !config_debug.show_profiler)
        profiler_visible = false;

    Profiler* profiler = emu_get_core()->GetProfiler();
    if (IsValidPointer(profiler))
        profiler->Enable(profiler_visible && !profiler_paused);
}

void gui_debug_profiler_reset(void)
{
    profiler_rows.clear();
    profiler_function_count = 0;
    profiler_dirty = true;
}

static void draw_profiler(Profiler* profiler)
{
    if (!IsValidPointer(profiler) || !IsValidPointer(profiler->GetFunctions()))
        return;

    profiler->Sync();

    if (ImGui::Button(profiler_paused ? "Resume" : "Pause"))
        profiler_paused = !profiler_paused;

    ImGui::SameLine();

    if (ImGui::Button("Reset"))
    {
        profiler->Reset();
        profiler_dirty = true;
    }

    const GB_Profiler_Function* functions = profiler->GetFunctions();
    u32 count = profiler->GetFunctionCount();
    u64 total = profiler->GetTotalCycles();

    ImGui::SameLine();
    ImGui::Text("Functions: %u  Frames: %llu  Cycles: %llu", count - 2,
        (unsigned long long)(total / GAMEBOY_CLOCKS_PER_FRAME), (unsigned long long)total);

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##profiler_filter", "Filter...", profiler_filter, IM_ARRAYSIZE(profiler_filter)))
        profiler_dirty = true;
    ImGui::PopItemWidth();

    ImGui::Separator();

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Hideable;
    ImGuiTableColumnFlags number_flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending;

    if (ImGui::BeginTable("profiler_table", ProfilerColumn_Count, flags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 2.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 66.0f);
        ImGui::TableSetupColumn("Bank", ImGuiTableColumnFlags_WidthFixed, 45.0f);
        ImGui::TableSetupColumn("Calls", number_flags, 45.0f);
        ImGui::TableSetupColumn("Calls/Frame", number_flags, 91.0f);
        ImGui::TableSetupColumn("Incl. Cycles", number_flags | ImGuiTableColumnFlags_DefaultSort, 87.0f);
        ImGui::TableSetupColumn("Incl. %", number_flags, 55.0f);
        ImGui::TableSetupColumn("Excl. Cycles", number_flags, 90.0f);
        ImGui::TableSetupColumn("Excl. %", number_flags, 58.0f);
        ImGui::TableSetupColumn("Avg", number_flags, 53.0f);
        ImGui::TableSetupColumn("Min", number_flags, 51.0f);
        ImGui::TableSetupColumn("Max", number_flags, 52.0f);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int column = 0; column < ProfilerColumn_Count; column++)
        {
            if (!ImGui::TableSetColumnIndex(column))
                continue;

            ImGui::PushID(column);
            ImGui::TableHeader(ImGui::TableGetColumnName(column));
            ImGui::PopID();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", k_profiler_column_tooltips[column]);
        }

        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        {
            if (sort_specs->SpecsDirty)
            {
                sort_specs->SpecsDirty = false;
                profiler_dirty = true;
            }

            if (sort_specs->SpecsCount > 0)
            {
                profiler_sort_column = sort_specs->Specs[0].ColumnIndex;
                profiler_sort_ascending = (sort_specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending);
            }
        }

        double time = ImGui::GetTime();

        if (profiler_dirty || (count != profiler_function_count) || ((time - profiler_refresh_time) >= PROFILER_REFRESH_SECONDS))
        {
            build_rows(functions, count);
            profiler_refresh_time = time;
        }

        ImGui::PushFont(gui_default_font);

        ImGuiListClipper clipper;
        clipper.Begin((int)profiler_rows.size());
        while (clipper.Step())
        {
            for (int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; idx++)
            {
                ProfilerRow& row = profiler_rows[idx];
                const GB_Profiler_Function& function = functions[row.index];
                bool pseudo = is_pseudo_function(function);
                bool root = (function.type == PROFILER_FUNCTION_ROOT);
                u32 completed = get_completed_calls(function);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                char selectable_id[32];
                snprintf(selectable_id, sizeof(selectable_id), "##prof%d", (int)row.index);
                if (ImGui::Selectable(selectable_id, false, ImGuiSelectableFlags_SpanAllColumns) && !pseudo)
                {
                    gui_debug_goto_address(function.address);
                }

                ImGui::PopFont();
                if (!pseudo && ImGui::BeginPopupContextItem())
                {
                    if (ImGui::Selectable("Add Breakpoint"))
                    {
                        Processor* processor = emu_get_core()->GetProcessor();
                        if (!processor->IsBreakpoint(Processor::GB_BREAKPOINT_TYPE_ROMRAM, function.address))
                            processor->AddBreakpoint(function.address);
                    }

                    ImGui::EndPopup();
                }
                ImGui::PushFont(gui_default_font);

                ImGui::SameLine(0, 0);
                if (pseudo)
                    ImGui::TextColored(orange, "%s", row.name);
                else if (row.has_symbol)
                    ImGui::TextColored(row.is_manual ? green : yellow, "%s", row.name);
                else
                    ImGui::TextColored(gray, "-");

                ImGui::TableNextColumn();
                if (pseudo)
                    ImGui::TextColored(gray, " ----");
                else
                    ImGui::TextColored(cyan, " %04X", function.address);

                ImGui::TableNextColumn();
                if (pseudo)
                    ImGui::TextColored(gray, " --");
                else
                    ImGui::TextColored(violet, " %02X", function.bank);

                ImGui::TableNextColumn();
                if (root)
                    draw_empty();
                else
                    draw_number(function.calls);

                ImGui::TableNextColumn();
                if (root || (total == 0))
                    draw_empty();
                else
                    draw_calls_per_frame(function.calls, total);

                ImGui::TableNextColumn();
                if (root)
                    draw_empty();
                else
                    draw_number(function.inclusive_cycles);

                ImGui::TableNextColumn();
                if (root)
                    draw_empty();
                else
                    draw_percent(function.inclusive_cycles, total);

                ImGui::TableNextColumn();
                draw_number(function.exclusive_cycles);

                ImGui::TableNextColumn();
                draw_percent(function.exclusive_cycles, total);

                ImGui::TableNextColumn();
                if (root || (completed == 0))
                    draw_empty();
                else
                    draw_number(function.inclusive_cycles / completed);

                ImGui::TableNextColumn();
                if (root || (completed == 0))
                    draw_empty();
                else
                    draw_number(function.min_cycles);

                ImGui::TableNextColumn();
                if (root || (completed == 0))
                    draw_empty();
                else
                    draw_number(function.max_cycles);
            }
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }
}

static void build_rows(const GB_Profiler_Function* functions, u32 count)
{
    char filter_upper[64] = { };
    for (int i = 0; i < 63 && profiler_filter[i]; i++)
        filter_upper[i] = (char)toupper(profiler_filter[i]);

    profiler_rows.clear();

    for (u32 i = 0; i < count; i++)
    {
        const GB_Profiler_Function& function = functions[i];

        ProfilerRow row;
        row.index = (u16)i;
        row.has_symbol = false;
        row.is_manual = false;
        row.name[0] = 0;

        if (function.type == PROFILER_FUNCTION_ROOT)
            snprintf(row.name, sizeof(row.name), "[Root]");
        else if (function.type == PROFILER_FUNCTION_HALT)
            snprintf(row.name, sizeof(row.name), "[HALT]");
        else
        {
            const char* name = gui_debug_get_symbol_name(function.bank, function.address, &row.is_manual);
            if (IsValidPointer(name))
            {
                row.has_symbol = true;
                snprintf(row.name, sizeof(row.name), "%s", name);
            }
        }

        if ((filter_upper[0] != 0) && !row_matches_filter(row, function, filter_upper))
            continue;

        profiler_rows.push_back(row);
    }

    profiler_sort_functions = functions;
    std::sort(profiler_rows.begin(), profiler_rows.end(), row_sort_compare);

    profiler_function_count = count;
    profiler_dirty = false;
}

static bool row_matches_filter(const ProfilerRow& row, const GB_Profiler_Function& function, const char* filter)
{
    char name_upper[64] = { };
    for (int i = 0; i < 63 && row.name[i]; i++)
        name_upper[i] = (char)toupper(row.name[i]);

    if (strstr(name_upper, filter) != NULL)
        return true;

    if (is_pseudo_function(function))
        return false;

    char address[8];
    snprintf(address, sizeof(address), "%04X", function.address);
    return strstr(address, filter) != NULL;
}

static bool is_pseudo_function(const GB_Profiler_Function& function)
{
    return (function.type == PROFILER_FUNCTION_ROOT) || (function.type == PROFILER_FUNCTION_HALT);
}

static u32 get_completed_calls(const GB_Profiler_Function& function)
{
    return (function.calls > function.active) ? function.calls - function.active : 0;
}

static u64 get_sort_value(const GB_Profiler_Function& function, int column)
{
    u32 completed = get_completed_calls(function);

    switch (column)
    {
        case ProfilerColumn_Bank:
            return ((u64)function.bank << 16) | function.address;
        case ProfilerColumn_Address:
            return function.address;
        case ProfilerColumn_Calls:
        case ProfilerColumn_CallsPerFrame:
            return function.calls;
        case ProfilerColumn_Inclusive:
        case ProfilerColumn_InclusivePercent:
            return function.inclusive_cycles;
        case ProfilerColumn_Exclusive:
        case ProfilerColumn_ExclusivePercent:
            return function.exclusive_cycles;
        case ProfilerColumn_Average:
            return (completed > 0) ? function.inclusive_cycles / completed : 0;
        case ProfilerColumn_Min:
            return (completed > 0) ? function.min_cycles : 0;
        case ProfilerColumn_Max:
            return (completed > 0) ? function.max_cycles : 0;
        default:
            return 0;
    }
}

static bool row_sort_compare(const ProfilerRow& a, const ProfilerRow& b)
{
    bool less = false;

    if (profiler_sort_column == ProfilerColumn_Function)
    {
        int result = strcmp(a.name, b.name);
        if (result == 0)
            return a.index < b.index;
        less = (result < 0);
    }
    else
    {
        u64 value_a = get_sort_value(profiler_sort_functions[a.index], profiler_sort_column);
        u64 value_b = get_sort_value(profiler_sort_functions[b.index], profiler_sort_column);
        if (value_a == value_b)
            return a.index < b.index;
        less = (value_a < value_b);
    }

    return profiler_sort_ascending ? less : !less;
}

static void draw_right_aligned(const ImVec4& color, const char* text)
{
    float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x;
    if (offset > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::TextColored(color, "%s", text);
}

static void draw_number(u64 value)
{
    char text[32];
    snprintf(text, sizeof(text), "%llu", (unsigned long long)value);
    draw_right_aligned(white, text);
}

static void draw_percent(u64 value, u64 total)
{
    char text[16];
    double percent = (total > 0) ? ((double)value * 100.0) / (double)total : 0.0;
    snprintf(text, sizeof(text), "%.2f%%", percent);
    draw_right_aligned(white, text);
}

static void draw_calls_per_frame(u32 calls, u64 total)
{
    char text[32];
    double frames = (double)total / (double)GAMEBOY_CLOCKS_PER_FRAME;
    snprintf(text, sizeof(text), "%.2f", (double)calls / frames);
    draw_right_aligned(white, text);
}

static void draw_empty(void)
{
    draw_right_aligned(gray, "-");
}
