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

#define GUI_DEBUG_DISASSEMBLER_IMPORT
#include "gui_debug_disassembler.h"

#include "imgui.h"
#include "fonts/IconsMaterialDesign.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui_debug_text.h"
#include "gui_debug_memory.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "config.h"
#include "emu.h"

struct DisassemblerLine
{
    u16 address;
    bool is_breakpoint;
    GS_Disassembler_Record* record;
    char name_enhanced[64];
    char tooltip[128];
    int name_real_length;
    DebugSymbol* symbol;
    bool is_auto_symbol;
};

struct DisassemblerBookmark
{
    u16 address;
    char name[32];
};

struct SymbolEntry
{
    DebugSymbol* symbol;
    bool is_manual;
    int bank;
};

static bool symbols_dirty = true;
static bool show_auto_symbols = false;
static DebugSymbol*** fixed_symbols = NULL;
static DebugSymbol*** dynamic_symbols = NULL;
static std::vector<SymbolEntry> fixed_symbol_list;
static std::vector<SymbolEntry> dynamic_symbol_list;
static std::vector<DisassemblerLine> disassembler_lines(0x10000);
static std::vector<DisassemblerBookmark> bookmarks;
static int selected_address = -1;
static int selected_bank = -1;
static int new_breakpoint_type = Processor::GS_BREAKPOINT_TYPE_ROMRAM;
static char new_breakpoint_buffer[10] = "";
static bool new_breakpoint_read = false;
static bool new_breakpoint_write = false;
static bool new_breakpoint_execute = true;
static char runto_address[5] = "";
static char goto_address[5] = "";
static bool goto_address_requested = false;
static u16 goto_address_target = 0;
static bool goto_back_requested = false;
static int goto_back = 0;
static int pc_pos = 0;
static int goto_address_pos = 0;
static bool add_bookmark_open = false;
static bool add_symbol_open = false;

static void draw_controls(void);
static void draw_breakpoints(void);
static void draw_breakpoints_content(void);
static void prepare_drawable_lines(void);
static void draw_disassembly(void);
static void draw_context_menu(DisassemblerLine* line);
static void add_debug_symbols();
static void add_symbol(const char* line);
static void add_breakpoint(int type);
static void request_goto_address(u16 addr);
static bool is_return_instruction(GS_Disassembler_Record* record);
static void replace_symbols(DisassemblerLine* line, const char* jump_color, const char* operand_color, const char* auto_color, const char* original_color);
static void replace_labels(DisassemblerLine* line, const char* color, const char* original_color);
static void draw_instruction_name(DisassemblerLine* line, bool is_pc);
static void disassembler_menu(void);
static void add_bookmark_popup(void);
static void add_symbol_popup(void);
static void save_full_disassembler(FILE* file);
static void save_current_disassembler(FILE* file);
static bool symbol_sort_address_asc(const SymbolEntry& a, const SymbolEntry& b);
static bool symbol_sort_address_desc(const SymbolEntry& a, const SymbolEntry& b);
static bool symbol_sort_addr_only_asc(const SymbolEntry& a, const SymbolEntry& b);
static bool symbol_sort_addr_only_desc(const SymbolEntry& a, const SymbolEntry& b);
static bool symbol_sort_name_asc(const SymbolEntry& a, const SymbolEntry& b);
static bool symbol_sort_name_desc(const SymbolEntry& a, const SymbolEntry& b);

void gui_debug_disassembler_init(void)
{
    fixed_symbols = new DebugSymbol**[0x100];
    dynamic_symbols = new DebugSymbol**[0x100];

    for (int i = 0; i < 0x100; i++)
    {
        fixed_symbols[i] = new DebugSymbol*[0x10000];
        dynamic_symbols[i] = new DebugSymbol*[0x10000];
    }

    for (int i = 0; i < 0x100; i++)
    {
        for (int j = 0; j < 0x10000; j++)
        {
            InitPointer(fixed_symbols[i][j]);
            InitPointer(dynamic_symbols[i][j]);
        }
    }
}

void gui_debug_disassembler_destroy(void)
{
    for (int i = 0; i < 0x100; i++)
    {
        for (int j = 0; j < 0x10000; j++)
        {
            SafeDelete(fixed_symbols[i][j]);
            SafeDelete(dynamic_symbols[i][j]);
        }

        SafeDeleteArray(fixed_symbols[i]);
        SafeDeleteArray(dynamic_symbols[i]);
    }

    SafeDeleteArray(fixed_symbols);
    SafeDeleteArray(dynamic_symbols);
}

void gui_debug_disassembler_reset(void)
{
    selected_address = -1;
    selected_bank = -1;
}

void gui_debug_reset_symbols(void)
{
    for (int i = 0; i < 0x100; i++)
    {
        for (int j = 0; j < 0x10000; j++)
        {
            SafeDelete(fixed_symbols[i][j]);
            SafeDelete(dynamic_symbols[i][j]);
        }
    }

    fixed_symbol_list.clear();
    dynamic_symbol_list.clear();
    symbols_dirty = true;

    add_debug_symbols();
}

void gui_debug_reset_breakpoints(void)
{
    emu_get_core()->GetProcessor()->ResetBreakpoints();
    new_breakpoint_buffer[0] = 0;
}

bool gui_debug_load_symbols_file(const char* file_path)
{
    std::ifstream file;
    open_ifstream_utf8(file, file_path, std::ios::in);

    if (!file.is_open())
    {
        Debug("Symbol file %s not found", file_path);
        return false;
    }

    Log("Loading symbol file %s", file_path);

    std::string line;
    bool valid_section = true;

    while (std::getline(file, line))
    {
        size_t comment = line.find_first_of(';');
        if (comment != std::string::npos)
            line = line.substr(0, comment);
        line = line.erase(0, line.find_first_not_of(" \t\r\n"));
        line = line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty())
            continue;

        if (line.find("[") != std::string::npos)
        {
            valid_section = false;
            if (line.find("[symbols]") != std::string::npos)
                valid_section = true;
            else if (line.find("[labels]") != std::string::npos)
                valid_section = true;

            continue;
        }

        if (valid_section)
            add_symbol(line.c_str());
    }

    file.close();
    return true;
}

void gui_debug_toggle_breakpoint(void)
{
    if (selected_address >= 0)
    {
        if (emu_get_core()->GetProcessor()->IsBreakpoint(Processor::GS_BREAKPOINT_TYPE_ROMRAM, selected_address))
            emu_get_core()->GetProcessor()->RemoveBreakpoint(Processor::GS_BREAKPOINT_TYPE_ROMRAM, selected_address);
        else
            emu_get_core()->GetProcessor()->AddBreakpoint(selected_address);
    }
}

void gui_debug_add_bookmark(void)
{
    add_bookmark_open = true;
}

void gui_debug_add_symbol(void)
{
    add_symbol_open = true;
}

void gui_debug_runtocursor(void)
{
    if (selected_address >= 0)
    {
        gui_debug_runto_address(selected_address);
    }
}

void gui_debug_runto_address(u16 address)
{
    emu_get_core()->GetProcessor()->AddRunToBreakpoint(address);
    emu_debug_continue();
}

void gui_debug_go_back(void)
{
    goto_back_requested = true;
}

void gui_debug_window_disassembler(void)
{
    add_bookmark_popup();
    add_symbol_popup();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(166, 26), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(458, 553), ImGuiCond_FirstUseEver);

    ImGui::Begin("Disassembler", &config_debug.show_disassembler, ImGuiWindowFlags_MenuBar);

    disassembler_menu();
    draw_controls();

    ImGui::Separator();

    draw_breakpoints();
    draw_disassembly();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_save_disassembler(const char* file_path, bool full)
{
    FILE* file = fopen_utf8(file_path, "w");

    if (IsValidPointer(file))
    {
        if (full)
            save_full_disassembler(file);
        else
            save_current_disassembler(file);
    }

    fclose(file);
}

static void draw_controls(void)
{
    ImGui::PushFont(gui_material_icons_font);

    if (ImGui::Button(ICON_MD_PLAY_ARROW))
    {
        emu_debug_continue();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Start / Continue (%s)", config_hotkeys[config_HotkeyIndex_DebugContinue].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_STOP))
    {
        emu_debug_break();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Stop (%s)", config_hotkeys[config_HotkeyIndex_DebugBreak].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_REDO))
    {
        emu_debug_step_over();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Step Over (%s)", config_hotkeys[config_HotkeyIndex_DebugStepOver].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_FILE_DOWNLOAD))
    {
        emu_debug_step_into();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Step Into (%s)", config_hotkeys[config_HotkeyIndex_DebugStepInto].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_FILE_UPLOAD))
    {
        emu_debug_step_out();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Step Out (%s)", config_hotkeys[config_HotkeyIndex_DebugStepOut].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_INPUT))
    {
        emu_debug_step_frame();
        gui_debug_memory_step_frame();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Step Frame (%s)", config_hotkeys[config_HotkeyIndex_DebugStepFrame].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_KEYBOARD_TAB))
    {
        gui_debug_runtocursor();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Run to Cursor (%s)", config_hotkeys[config_HotkeyIndex_DebugRunToCursor].str);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_REPLAY))
    {
        emu_reset(config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Reset (%s)", config_hotkeys[config_HotkeyIndex_Reset].str);
    }

    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::TextColored(emu_is_debug_idle() ? red : green, emu_is_debug_idle() ? "   PAUSED" : "   RUNNING");
}

static const char* k_breakpoint_types[] = { "ROM/RAM ", "VRAM    ", "IO      " };

static void draw_breakpoints_content(void)
{

    ImGui::Checkbox("Break On IRQs##irq_break", &emu_debug_irq_breakpoints); ImGui::SameLine();
    ImGui::Checkbox("Disable All##disable_mem", &emu_debug_disable_breakpoints); ImGui::SameLine();

    if (ImGui::Button("Remove All##clear_all", ImVec2(85, 0)))
    {
        gui_debug_reset_breakpoints();
    }

    ImGui::Columns(2, "breakpoints");
    ImGui::SetColumnOffset(1, 130);

    ImGui::Separator();

    ImGui::PushItemWidth(120);
    ImGui::Combo("Type##type", &new_breakpoint_type, "ROM/RAM\0VRAM\0IO\0");

    ImGui::PushItemWidth(85);
    if (ImGui::InputTextWithHint("##add_breakpoint", "XXXX-XXXX", new_breakpoint_buffer, IM_ARRAYSIZE(new_breakpoint_buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
    {
        add_breakpoint(new_breakpoint_type);
    }
    ImGui::PopItemWidth();

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Use hex XXXX format for single addresses or XXXX-XXXX for address ranges");

    ImGui::Checkbox("Read", &new_breakpoint_read);
    ImGui::Checkbox("Write", &new_breakpoint_write);

    if (new_breakpoint_type == Processor::GS_BREAKPOINT_TYPE_ROMRAM)
        ImGui::Checkbox("Execute", &new_breakpoint_execute);

    if (ImGui::Button("Add##add", ImVec2(85, 0)))
    {
        add_breakpoint(new_breakpoint_type);
    }

    ImGui::NextColumn();

    ImGui::BeginChild("breakpoints", ImVec2(0, 130), false);
    ImGui::PushFont(gui_default_font);

    int remove = -1;
    std::vector<Processor::GS_Breakpoint>* breakpoints = emu_get_core()->GetProcessor()->GetBreakpoints();

    for (long unsigned int b = 0; b < breakpoints->size(); b++)
    {
        Processor::GS_Breakpoint* brk = &(*breakpoints)[b];

        ImGui::PushID(10000 + b);
        if (ImGui::SmallButton("X"))
        {
           remove = b;
           ImGui::PopID();
           continue;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Remove breakpoint");
            ImGui::EndTooltip();
        }

        ImGui::PopID();

        ImGui::SameLine();

        ImGui::PushID(20000 + b);
        if (ImGui::SmallButton(brk->enabled ? "-" : "+"))
        {
            brk->enabled = !brk->enabled;
        }
        ImGui::PopID();
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text(brk->enabled ? "Disable breakpoint" : "Enable breakpoint");
            ImGui::EndTooltip();
        }

        ImGui::SameLine(); ImGui::TextColored(brk->enabled ? red : gray, "%s", k_breakpoint_types[brk->type]); ImGui::SameLine(0, 0);

        if ((*breakpoints)[b].range)
            ImGui::TextColored(brk->enabled ? cyan : gray, "%04X-%04X", brk->address1, brk->address2);
        else
            ImGui::TextColored(brk->enabled ? cyan : gray, "%04X", brk->address1);

        ImGui::SameLine(0, 0); ImGui::TextColored(brk->enabled && brk->read ? orange : gray, " R");
        ImGui::SameLine(0, 2); ImGui::TextColored(brk->enabled && brk->write ? orange : gray, "W");

        if (brk->type == Processor::GS_BREAKPOINT_TYPE_ROMRAM)
        {
            ImGui::SameLine(0, 2); ImGui::TextColored(brk->enabled && brk->execute ? orange : gray, "X");
        }

        GS_Disassembler_Record* record = emu_get_core()->GetMemory()->GetDisassemblerRecord(brk->address1);

        bool symbol_shown = false;

        if (!brk->range && (brk->type == Processor::GS_BREAKPOINT_TYPE_ROMRAM) && IsValidPointer(record))
        {
            DebugSymbol* symbol = fixed_symbols[record->bank][brk->address1];
            if (!IsValidPointer(symbol))
                symbol = dynamic_symbols[record->bank][brk->address1];
            if (IsValidPointer(symbol))
            {
                ImGui::SameLine(0, 0);
                ImGui::TextColored(brk->enabled ? green : gray, " %s", symbol->text);
                symbol_shown = true;
            }
            else if (record->auto_symbol[0] != 0)
            {
                ImGui::SameLine(0, 0);
                ImGui::TextColored(brk->enabled ? green : gray, " %s", record->auto_symbol);
                symbol_shown = true;
            }
        }

        if (!symbol_shown && brk->execute && IsValidPointer(record))
        {
            ImGui::SameLine(0, 0);
            ImGui::PushStyleColor(ImGuiCol_Text, brk->enabled ? white : gray);
            TextColoredEx(" %s", record->name);
            ImGui::PopStyleColor();
        }
    }

    ImGui::PopFont();

    if (remove >= 0)
    {
        breakpoints->erase(breakpoints->begin() + remove);
    }

    ImGui::EndChild();
    ImGui::Columns(1);
    ImGui::Separator();

}

static void draw_breakpoints(void)
{
    if (ImGui::CollapsingHeader("Breakpoints"))
    {
        draw_breakpoints_content();
    }
}

void gui_debug_window_breakpoints(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(408, 264), ImGuiCond_FirstUseEver);

    ImGui::Begin("Breakpoints", &config_debug.show_breakpoints);

    draw_breakpoints_content();

    ImGui::End();
    ImGui::PopStyleVar();
}

static void prepare_drawable_lines(void)
{
    Memory* memory = emu_get_core()->GetMemory();
    Processor* processor = emu_get_core()->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    u16 pc = proc_state->PC->GetValue();

    disassembler_lines.clear();
    pc_pos = 0;
    goto_address_pos = 0;

    for (int i = 0; i < 0x10000; i++)
    {
        GS_Disassembler_Record* record = memory->GetDisassemblerRecord(i);

        if (IsValidPointer(record) && (record->name[0] != 0))
        {
            if (record->auto_symbol[0] != 0)
            {
                DebugSymbol* existing = dynamic_symbols[record->bank][i];
                if (!IsValidPointer(existing))
                {
                    existing = new DebugSymbol;
                    existing->address = (u16)i;
                    existing->bank = record->bank;
                    snprintf(existing->text, 64, "%s", record->auto_symbol);
                    dynamic_symbols[record->bank][i] = existing;

                    SymbolEntry entry;
                    entry.symbol = existing;
                    entry.is_manual = false;
                    entry.bank = record->bank;
                    dynamic_symbol_list.push_back(entry);

                    if (show_auto_symbols)
                        symbols_dirty = true;
                }
                else if (strcmp(existing->text, record->auto_symbol) != 0)
                {
                    if (strncmp(existing->text, "SUB_", 4) == 0 || strncmp(existing->text, "TAG_", 4) == 0)
                    {
                        snprintf(existing->text, 64, "%s", record->auto_symbol);
                        if (show_auto_symbols)
                            symbols_dirty = true;
                    }
                }
            }

            bool fixed_symbol_found = false;
            if (config_debug.dis_show_symbols)
            {
                DebugSymbol* symbol = fixed_symbols[record->bank][i];

                if (IsValidPointer(symbol))
                {
                    DisassemblerLine line;
                    line.address = (u16)i;
                    line.symbol = symbol;
                    line.is_auto_symbol = false;
                    disassembler_lines.push_back(line);
                    fixed_symbol_found = true;
                }
            }

            if (config_debug.dis_show_symbols && config_debug.dis_show_auto_symbols && !fixed_symbol_found)
            {
                DebugSymbol* symbol = dynamic_symbols[record->bank][i];

                if (IsValidPointer(symbol))
                {
                    DisassemblerLine line;
                    line.address = (u16)i;
                    line.symbol = symbol;
                    line.is_auto_symbol = true;
                    disassembler_lines.push_back(line);
                }
            }

            DisassemblerLine line;
            line.address = (u16)i;
            line.symbol = NULL;
            line.is_breakpoint = false;
            line.record = record;
            snprintf(line.name_enhanced, 64, "%s", line.record->name);
            line.tooltip[0] = 0;

            std::vector<Processor::GS_Breakpoint>* breakpoints = emu_get_core()->GetProcessor()->GetBreakpoints();

            for (long unsigned int b = 0; b < breakpoints->size(); b++)
            {
                Processor::GS_Breakpoint* brk = &(*breakpoints)[b];

                if (brk->execute && (brk->address1 == i))
                {
                    line.is_breakpoint = true;
                    break;
                }
            }

            if ((u16)i == pc)
                pc_pos = (int)disassembler_lines.size();

            if (goto_address_requested && (i <= goto_address_target))
            {
                goto_address_pos = (int)disassembler_lines.size();
                if ((goto_address_pos > 0) && disassembler_lines[goto_address_pos - 1].symbol)
                    goto_address_pos--;
            }

            disassembler_lines.push_back(line);
        }
    }
}

static void draw_disassembly(void)
{
    ImGui::PushFont(gui_default_font);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, mid_gray);

    bool window_visible = ImGui::BeginChild("##dis", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (window_visible)
    {
        Processor* processor = emu_get_core()->GetProcessor();
        Processor::ProcessorState* proc_state = processor->GetState();
        u16 pc = proc_state->PC->GetValue();

        prepare_drawable_lines();

        if (emu_debug_pc_changed)
        {
            emu_debug_pc_changed = false;
            float window_offset = ImGui::GetWindowHeight() / 2.0f;
            float offset = window_offset - (ImGui::GetTextLineHeightWithSpacing() - 2.0f);
            ImGui::SetScrollY((pc_pos * ImGui::GetTextLineHeightWithSpacing()) - offset);
        }

        if (goto_address_requested)
        {
            goto_address_requested = false;
            goto_back = (int)ImGui::GetScrollY();
            ImGui::SetScrollY((goto_address_pos * ImGui::GetTextLineHeightWithSpacing()) + 2);
        }

        if (goto_back_requested)
        {
            goto_back_requested = false;
            ImGui::SetScrollY((float)goto_back);
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)disassembler_lines.size(), ImGui::GetTextLineHeightWithSpacing());

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                DisassemblerLine line = disassembler_lines[item];

                if (line.symbol)
                {
                    bool dim = line.is_auto_symbol && config_debug.dis_dim_auto_symbols;
                    ImGui::TextColored(dim ? dim_green : green, "%s:", line.symbol->text);
                    continue;
                }

                ImGui::PushID(item);

                bool is_selected = (selected_address == line.address);

                if (ImGui::Selectable("", is_selected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(0) && line.record->jump)
                    {
                        request_goto_address(line.record->jump_address);
                    }
                    else if (is_selected)
                    {
                        selected_address = -1;
                        selected_bank = -1;
                        new_breakpoint_buffer[0] = 0;
                    }
                    else
                    {
                        selected_address = line.address;
                        selected_bank = line.record->bank;
                    }
                }

                bool enable_bg_color = false;
                ImVec4 bg_color;

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                else if (line.is_breakpoint && !ImGui::IsItemHovered())
                {
                    enable_bg_color = true;
                    bg_color = dark_red;
                }
                else if ((line.address == pc) && !ImGui::IsItemHovered())
                {
                    enable_bg_color = true;
                    bg_color = dark_yellow;
                }
                else if (line.record->subroutine && !ImGui::IsItemHovered())
                {
                    enable_bg_color = true;
                    bg_color = dark_gray;
                }

                if (enable_bg_color)
                {
                    ImVec2 p_min = ImGui::GetItemRectMin();
                    ImVec2 p_max = ImGui::GetItemRectMax();
                    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImGui::GetColorU32(bg_color));
                }

                draw_context_menu(&line);

                ImVec4 color_segment = line.is_breakpoint ? red : magenta;
                ImVec4 color_bank = line.is_breakpoint ? red : violet;
                ImVec4 color_addr = line.is_breakpoint ? red : cyan;
                ImVec4 color_mem = line.is_breakpoint ? red : mid_gray;

                if (config_debug.dis_show_segment)
                {
                    ImGui::SameLine();
                    ImGui::TextColored(color_segment, "%s", line.record->segment);
                }

                if (config_debug.dis_show_bank)
                {
                    ImGui::SameLine();
                    ImGui::TextColored(color_bank, "%02X", line.record->bank);
                }

                ImGui::SameLine();
                ImGui::TextColored(color_addr, "%04X", line.address);

                ImGui::SameLine();
                if (line.address == pc)
                {
                    ImGui::TextColored(yellow, " ->");
                }
                else
                {
                    ImGui::TextColored(yellow, "   ");
                }

                ImGui::SameLine();
                draw_instruction_name(&line, line.address == pc);

                if (line.tooltip[0] != 0 && ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    TextColoredEx("%s", line.tooltip);
                    ImGui::EndTooltip();
                }

                if (config_debug.dis_show_mem)
                {
                    int len = line.name_real_length;
                    char spaces[32];
                    int offset = 28 - len;
                    if (offset < 0)
                        offset = 0;
                    for (int i = 0; i < offset; i++)
                        spaces[i] = ' ';
                    spaces[offset] = 0;
                    ImGui::SameLine();
                    ImGui::TextColored(color_mem, "%s;%s", spaces, line.record->bytes);
                }

                bool is_ret = is_return_instruction(line.record);
                if (is_ret)
                {
                    ImGui::PushStyleColor(ImGuiCol_Separator, dark_green);
                    ImGui::Separator();
                    ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }
        }
    }

    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopFont();

}

static void draw_context_menu(DisassemblerLine* line)
{
    ImGui::PopFont();
    if (ImGui::BeginPopupContextItem())
    {
        selected_address = line->address;
        selected_bank = line->record->bank;

        if (ImGui::Selectable("Run To Cursor"))
        {
            gui_debug_runtocursor();
        }

        if (ImGui::Selectable("Add Bookmark..."))
        {
            gui_debug_add_bookmark();
        }

        if (ImGui::Selectable("Add Symbol..."))
        {
            gui_debug_add_symbol();
        }

        if (ImGui::Selectable("Toggle Breakpoint"))
        {
            gui_debug_toggle_breakpoint();
        }

        ImGui::EndPopup();
    }
    ImGui::PushFont(gui_default_font);
}

static void add_debug_symbols()
{
    for (int i = 0; i < k_debug_symbol_count; i++)
    {
        u16 address = k_debug_symbols[i].address;
        u8 bank = 0;

        DebugSymbol* existing = dynamic_symbols[bank][address];
        if (!IsValidPointer(existing))
        {
            DebugSymbol* new_symbol = new DebugSymbol;
            new_symbol->address = address;
            new_symbol->bank = bank;
            snprintf(new_symbol->text, 64, "%s", k_debug_symbols[i].label);

            dynamic_symbols[bank][address] = new_symbol;

            SymbolEntry entry;
            entry.symbol = new_symbol;
            entry.is_manual = false;
            entry.bank = bank;
            dynamic_symbol_list.push_back(entry);
        }
    }
    symbols_dirty = true;
}

static void add_symbol(const char* line)
{
    Debug("Loading symbol %s", line);

    DebugSymbol s;
    std::string str(line);

    // Clean up the string
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    std::replace(str.begin(), str.end(), '\t', ' ');

    // Trim leading/trailing whitespace
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
        str = "";
    else
    {
        size_t last = str.find_last_not_of(' ');
        str = str.substr(first, (last - first + 1));
    }

    // Remove comments
    std::size_t comment = str.find(";");
    if (comment != std::string::npos)
        str = str.substr(0, comment);

    // Tokenize the string
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() < 2)
        return;

    std::string bank_str = "0";
    std::string addr_str;
    std::string symbol;
    bool parsed = false;

    // EQU format: <symbol>[:]  EQU  <value>
    // e.g. "label: EQU 0x00004000" or "label EQU 1234h"
    if (!parsed && tokens.size() >= 3)
    {
        std::string equ_upper = tokens[1];
        for (size_t c = 0; c < equ_upper.size(); c++) equ_upper[c] = toupper(equ_upper[c]);

        if (equ_upper == "EQU")
        {
            std::string sym_name = tokens[0];
            if (!sym_name.empty() && sym_name.back() == ':')
                sym_name = sym_name.substr(0, sym_name.length() - 1);

            if (!sym_name.empty())
            {
                std::string value = tokens[2];
                if (value.length() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                    value = value.substr(2);
                if (!value.empty() && (value.back() == 'h' || value.back() == 'H'))
                    value = value.substr(0, value.length() - 1);

                if (value.length() > 4)
                {
                    bank_str = value.substr(0, value.length() - 4);
                    addr_str = value.substr(value.length() - 4);
                }
                else
                {
                    addr_str = value;
                }

                symbol = sym_name;
                parsed = true;
            }
        }
    }

    // GBDK/SDCC/NoICE (.noi) format: DEF <symbol> <address>
    // e.g. "DEF _main 0100" or "def bar 234h"
    if (!parsed && tokens.size() >= 3)
    {
        std::string keyword_upper = tokens[0];
        for (size_t c = 0; c < keyword_upper.size(); c++) keyword_upper[c] = toupper(keyword_upper[c]);

        if (keyword_upper == "DEF")
        {
            std::string value = tokens[2];
            if (value.length() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                value = value.substr(2);
            if (!value.empty() && (value.back() == 'h' || value.back() == 'H'))
                value = value.substr(0, value.length() - 1);

            addr_str = value;
            symbol = tokens[1];
            parsed = true;
        }
    }

    // RGBDS/WLA-DX/no$gmb format: <bank:address> <symbol>
    // e.g. "00:0150 Start" or "01:4000 BankedFunc"
    if (!parsed && tokens.size() >= 2 && tokens[0].find(':') != std::string::npos)
    {
        std::string addr_part = tokens[0];
        std::size_t separator = addr_part.find(":");

        bank_str = addr_part.substr(0, separator);
        addr_str = addr_part.substr(separator + 1);
        symbol = tokens[1];
        parsed = true;
    }

    // Generic format: <address> <symbol>
    // e.g. "0150 Start"
    // Also matches RGBDS exported constants: <hex_value> <symbol>
    // Skip values > 4 hex digits (constants, not addresses)
    if (!parsed && tokens.size() >= 2)
    {
        if (tokens[0].length() <= 4)
        {
            addr_str = tokens[0];
            symbol = tokens[1];
            parsed = true;
        }
    }

    if (!parsed)
        return;

    // Parse the bank and address values
    u16 bank_value = 0;
    if (parse_hex_string(bank_str.c_str(), bank_str.length(), &bank_value))
    {
        u16 address_value = 0;
        if (parse_hex_string(addr_str.c_str(), addr_str.length(), &address_value))
        {
            s.bank = bank_value;
            s.address = address_value;
            snprintf(s.text, 64, "%s", symbol.c_str());

            // Store the symbol
            DebugSymbol* existing = fixed_symbols[s.bank][s.address];
            if (IsValidPointer(existing))
            {
                for (size_t i = 0; i < fixed_symbol_list.size(); i++)
                {
                    if (fixed_symbol_list[i].symbol == existing)
                    {
                        fixed_symbol_list.erase(fixed_symbol_list.begin() + i);
                        break;
                    }
                }
                SafeDelete(fixed_symbols[s.bank][s.address]);
            }

            DebugSymbol* new_symbol = new DebugSymbol;
            new_symbol->address = s.address;
            new_symbol->bank = s.bank;
            snprintf(new_symbol->text, 64, "%s", s.text);

            fixed_symbols[s.bank][s.address] = new_symbol;

            SymbolEntry entry;
            entry.symbol = new_symbol;
            entry.is_manual = true;
            entry.bank = s.bank;
            fixed_symbol_list.push_back(entry);
            symbols_dirty = true;
        }
    }
}

static void add_breakpoint(int type)
{
    bool read = new_breakpoint_read;
    bool write = new_breakpoint_write;
    bool execute = new_breakpoint_execute;

    if (type != Processor::GS_BREAKPOINT_TYPE_ROMRAM)
    {
        if (!read && !write)
            return;
        execute = false;
    }

    if (emu_get_core()->GetProcessor()->AddBreakpoint(type, new_breakpoint_buffer, read, write, execute))
        new_breakpoint_buffer[0] = 0;
}

static void request_goto_address(u16 address)
{
    goto_address_requested = true;
    goto_address_target = address;
}

static bool is_return_instruction(GS_Disassembler_Record* record)
{
    if (!IsValidPointer(record) || record->size == 0)
        return false;

    u8 opcode = record->opcodes[0];

    switch (opcode)
    {
        case 0xC9: // RET
        case 0xC0: // RET NZ
        case 0xC8: // RET Z
        case 0xD0: // RET NC
        case 0xD8: // RET C
        case 0xD9: // RETI
            return true;
        default:
            return false;
    }
}

static bool replace_address_in_string(std::string& instr, u16 address, bool is_zp, const char* replacement_text)
{
    const char* format = is_zp ? "$%02X" : "$%04X";
    const char* indirect_format = is_zp ? "$(%02X" : "$(%04X";
    int replace_len = is_zp ? 3 : 5;
    int indirect_replace_len = is_zp ? 4 : 6;

    char address_str[8];
    snprintf(address_str, 8, format, address);
    size_t pos = instr.find(address_str);
    if (pos != std::string::npos)
    {
        instr.replace(pos, replace_len, replacement_text);
        return true;
    }

    snprintf(address_str, 8, indirect_format, address);
    pos = instr.find(address_str);
    if (pos != std::string::npos)
    {
        std::string indirect_replacement = std::string("(") + replacement_text;
        instr.replace(pos, indirect_replace_len, indirect_replacement);
        return true;
    }

    return false;
}

static bool get_record_operand(GS_Disassembler_Record* record, u16* out_address, bool* out_is_zp)
{
    if (record->jump)
    {
        *out_address = record->jump_address;
        *out_is_zp = false;
        return true;
    }
    else if (record->has_operand_address)
    {
        *out_address = record->operand_address;
        *out_is_zp = false;
        return true;
    }

    return false;
}

bool gui_debug_resolve_symbol(GS_Disassembler_Record* record, std::string& instr, const char* color, const char* original_color, const char** out_name, u16* out_address)
{
    u16 lookup_address = 0;
    bool is_zp = false;

    if (!get_record_operand(record, &lookup_address, &is_zp))
        return false;

    u8 bank = record->jump ? record->jump_bank : emu_get_core()->GetMemory()->GetBank(lookup_address);
    DebugSymbol* symbol = fixed_symbols[bank][lookup_address];
    if (IsValidPointer(symbol))
    {
        std::string replacement = std::string(color) + symbol->text + original_color;
        if (replace_address_in_string(instr, lookup_address, is_zp, replacement.c_str()))
        {
            if (out_name) *out_name = symbol->text;
            if (out_address) *out_address = lookup_address;
            return true;
        }
    }

    return false;
}

bool gui_debug_resolve_label(GS_Disassembler_Record* record, std::string& instr, const char* color, const char* original_color, const char** out_name, u16* out_address)
{
    u8 opcode = record->opcodes[0];
    bool is_ldh_out = (opcode == 0xE0);
    bool is_ldh_in = (opcode == 0xF0);

    if (is_ldh_out || is_ldh_in)
    {
        u16 port = record->opcodes[1];
        int dir = is_ldh_in ? IO_IN : IO_OUT;

        for (int i = 0; i < k_debug_io_label_count; i++)
        {
            if (k_debug_io_labels[i].address == port && (k_debug_io_labels[i].direction & dir))
            {
                char label_address[4];
                snprintf(label_address, 4, "$%02X", port);
                std::string replacement = std::string(color) + k_debug_io_labels[i].label + "_" + label_address + original_color;
                if (replace_address_in_string(instr, port, true, replacement.c_str()))
                {
                    if (out_name) *out_name = k_debug_io_labels[i].label;
                    if (out_address) *out_address = port;
                    return true;
                }
            }
        }
    }

    return false;
}

static void replace_symbols(DisassemblerLine* line, const char* jump_color, const char* operand_color, const char* auto_color, const char* original_color)
{
    std::string instr = line->record->name;
    const char* color = line->record->jump ? jump_color : operand_color;
    const char* resolved_name = NULL;
    u16 resolved_address = 0;

    if (gui_debug_resolve_symbol(line->record, instr, color, original_color, &resolved_name, &resolved_address))
    {
        snprintf(line->name_enhanced, 64, "%s", instr.c_str());
        snprintf(line->tooltip, 128, "%s%s%s = %s$%04X", color, resolved_name, c_white, c_cyan, resolved_address);
        return;
    }

    if (!config_debug.dis_show_auto_symbols)
        return;

    if (!line->record->jump)
        return;

    u16 lookup_address = 0;
    bool is_zp = false;

    if (!get_record_operand(line->record, &lookup_address, &is_zp))
        return;

    DebugSymbol* dynamic_symbol = dynamic_symbols[line->record->jump_bank][lookup_address];

    const char* auto_symbol_text = NULL;
    if (IsValidPointer(dynamic_symbol))
    {
        auto_symbol_text = dynamic_symbol->text;
    }
    else
    {
        GS_Disassembler_Record* target = emu_get_core()->GetMemory()->GetDisassemblerRecord(lookup_address, line->record->jump_bank);
        if (IsValidPointer(target) && target->auto_symbol[0] != 0)
            auto_symbol_text = target->auto_symbol;
    }

    if (auto_symbol_text != NULL)
    {
        std::string replacement = std::string(auto_color) + auto_symbol_text + original_color;
        if (replace_address_in_string(instr, lookup_address, is_zp, replacement.c_str()))
        {
            snprintf(line->name_enhanced, 64, "%s", instr.c_str());
            snprintf(line->tooltip, 128, "%s%s%s = %s$%04X", auto_color, auto_symbol_text, c_white, c_cyan, lookup_address);
        }
    }
}

static void replace_labels(DisassemblerLine* line, const char* color, const char* original_color)
{
    std::string instr = line->record->name;
    const char* resolved_name = NULL;
    u16 resolved_address = 0;

    if (gui_debug_resolve_label(line->record, instr, color, original_color, &resolved_name, &resolved_address))
    {
        snprintf(line->name_enhanced, 64, "%s", instr.c_str());
        if (line->tooltip[0] == 0)
            snprintf(line->tooltip, 128, "%s%s%s = %s$%04X", color, resolved_name, c_white, c_cyan, resolved_address);
    }
}

static void draw_instruction_name(DisassemblerLine* line, bool is_pc)
{
    const char* name_color;
    const char* operands_color;
    const char* symbol_color;
    const char* label_color;
    const char* extra_color;

    if (is_pc)
    {
        name_color = c_yellow;
        operands_color = c_yellow;
        symbol_color = c_yellow;
        label_color = c_yellow;
        extra_color = c_yellow;
    }
    else if (line->is_breakpoint)
    {
        name_color = c_red;
        operands_color = c_red;
        symbol_color = c_red;
        label_color = c_red;
        extra_color = c_red;
    }
    else
    {
        name_color = c_white;
        operands_color = c_brown;
        symbol_color = c_green;
        label_color = c_orange;
        extra_color = c_blue;
    }

    if (config_debug.dis_replace_symbols)
    {
        const char* auto_symbol_color = config_debug.dis_dim_auto_symbols ? c_dim_green : symbol_color;
        replace_symbols(line, symbol_color, label_color, auto_symbol_color, operands_color);
    }

    if (config_debug.dis_replace_labels)
    {
        replace_labels(line, label_color, operands_color);
    }

    std::string instr = line->name_enhanced;
    size_t pos = instr.find("{n}");
    if (pos != std::string::npos)
        instr.replace(pos, 3, name_color);
    pos = instr.find("{e}");
    if (pos != std::string::npos)
        instr.replace(pos, 3, extra_color);
    pos = instr.find("{o}");
    if (pos != std::string::npos)
        instr.replace(pos, 3, operands_color);

    ImGui::BeginGroup();
    line->name_real_length = TextColoredEx("%s%s", name_color, instr.c_str());
    ImGui::EndGroup();
}

static void disassembler_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save All Disassembled Code As..."))
        {
            gui_file_dialog_save_disassembler(true);
        }

        if (ImGui::MenuItem("Save Current View As..."))
        {
            gui_file_dialog_save_disassembler(false);
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("Opcodes", NULL, &config_debug.dis_show_mem);
        ImGui::MenuItem("Symbols", NULL, &config_debug.dis_show_symbols);
        ImGui::MenuItem("Segment", NULL, &config_debug.dis_show_segment);
        ImGui::MenuItem("Bank", NULL, &config_debug.dis_show_bank);

        ImGui::Separator();

        if (ImGui::BeginMenu("Run Ahead"))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::SliderInt("##lookahead", &config_debug.dis_look_ahead_count, 0, 100, "%d instructions");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Go"))
    {
        if (ImGui::MenuItem("Back", config_hotkeys[config_HotkeyIndex_DebugGoBack].str))
        {
            gui_debug_go_back();
        }

        if (ImGui::MenuItem("Go To PC"))
        {
            Processor* processor = emu_get_core()->GetProcessor();
            Processor::ProcessorState* proc_state = processor->GetState();
            u16 pc = proc_state->PC->GetValue();
            request_goto_address(pc);
        }

        if (ImGui::BeginMenu("Go To Address..."))
        {
            bool go = false;
            ImGui::PushItemWidth(45);
            if (ImGui::InputTextWithHint("##goto_address", "XXXX", goto_address, IM_ARRAYSIZE(goto_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
                go = true;

            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button("Go!", ImVec2(40, 0)))
                go = true;

            if (go)
            {
                u16 address_value = 0;
                if (parse_hex_string(goto_address, strlen(goto_address), &address_value))
                {
                    request_goto_address(address_value);
                }
                goto_address[0] = 0;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }


    if (ImGui::BeginMenu("Run"))
    {
        if (ImGui::MenuItem("Start", config_hotkeys[config_HotkeyIndex_DebugContinue].str))
        {
            emu_debug_continue();
        }

        if (ImGui::MenuItem("Stop", config_hotkeys[config_HotkeyIndex_DebugBreak].str))
        {
            emu_debug_break();
        }

        if (ImGui::MenuItem("Step Over", config_hotkeys[config_HotkeyIndex_DebugStepOver].str))
        {
            emu_debug_step_over();
        }

        if (ImGui::MenuItem("Step Into", config_hotkeys[config_HotkeyIndex_DebugStepInto].str))
        {
            emu_debug_step_into();
        }

        if (ImGui::MenuItem("Step Out", config_hotkeys[config_HotkeyIndex_DebugStepOut].str))
        {
            emu_debug_step_out();
        }

        if (ImGui::MenuItem("Step Frame", config_hotkeys[config_HotkeyIndex_DebugStepFrame].str))
        {
            emu_debug_step_frame();
            gui_debug_memory_step_frame();
        }

        if (ImGui::MenuItem("Run to Cursor", config_hotkeys[config_HotkeyIndex_DebugRunToCursor].str))
        {
            gui_debug_runtocursor();
        }

        if (ImGui::MenuItem("Reset", config_hotkeys[config_HotkeyIndex_Reset].str))
        {
            emu_reset(config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Run To Address..."))
        {
            bool go = false;
            ImGui::PushItemWidth(45);
            if (ImGui::InputTextWithHint("##runto_address", "XXXX", runto_address, IM_ARRAYSIZE(runto_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
                go = true;

            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button("Run!", ImVec2(50, 0)))
                go = true;

            if (go)
            {
                u16 address_value = 0;
                if (parse_hex_string(runto_address, strlen(runto_address), &address_value))
                {
                    gui_debug_runto_address(address_value);
                }
                runto_address[0] = 0;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Breakpoints"))
    {
        ImGui::MenuItem("Breakpoints Window", NULL, &config_debug.show_breakpoints);

        ImGui::Separator();

        if (ImGui::MenuItem("Toggle Selected Line", config_hotkeys[config_HotkeyIndex_DebugBreakpoint].str))
        {
            gui_debug_toggle_breakpoint();
        }

        ImGui::MenuItem("Break On IRQs", 0, &emu_debug_irq_breakpoints);

        ImGui::Separator();

        if (ImGui::MenuItem("Remove All"))
        {
            gui_debug_reset_breakpoints();
        }

        ImGui::MenuItem("Disable All", 0, &emu_debug_disable_breakpoints);

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Bookmarks"))
    {
        if (ImGui::MenuItem("Add Bookmark..."))
        {
            gui_debug_add_bookmark();
        }

        if (ImGui::MenuItem("Remove All"))
        {
            bookmarks.clear();
        }

        if (bookmarks.size() > 0)
            ImGui::Separator();

        for (long unsigned int i = 0; i < bookmarks.size(); i++)
        {
            char label[80];
            snprintf(label, 80, "$%04X: %s", bookmarks[i].address, bookmarks[i].name);
            if (ImGui::MenuItem(label))
            {
                request_goto_address(bookmarks[i].address);
            }
        }

        ImGui::EndMenu();
    }

    bool open_symbols = false;

    if (ImGui::BeginMenu("Symbols"))
    {
        ImGui::MenuItem("Symbols Window", NULL, &config_debug.show_symbols);

        ImGui::Separator();
        ImGui::MenuItem("Hardware Labels", NULL, &config_debug.dis_replace_labels);

        ImGui::MenuItem("Automatic Symbols", NULL, &config_debug.dis_show_auto_symbols);
        if (!config_debug.dis_show_auto_symbols) ImGui::BeginDisabled();
        ImGui::MenuItem("Dim Automatic Symbols", NULL, &config_debug.dis_dim_auto_symbols);
        if (!config_debug.dis_show_auto_symbols) ImGui::EndDisabled();
        ImGui::MenuItem("Replace Address With Symbol", NULL, &config_debug.dis_replace_symbols);

        ImGui::Separator();

        if (ImGui::MenuItem("Add Symbol..."))
        {
            gui_debug_add_symbol();
        }

        if (ImGui::MenuItem("Load Symbols..."))
        {
            open_symbols = true;
        }

        if (ImGui::MenuItem("Clear Symbols"))
        {
            gui_debug_reset_symbols();
        }

        ImGui::EndMenu();
    }

    if (open_symbols)
        gui_file_dialog_load_symbols();

    ImGui::EndMenuBar();
}

static void add_bookmark_popup(void)
{
    if (add_bookmark_open)
    {
        ImGui::OpenPopup("Add Bookmark");
        add_bookmark_open = false;
    }

    if (ImGui::BeginPopupModal("Add Bookmark", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char address_bookmark[5] = "";
        static char name_bookmark[32] = "";
        static bool bookmark_modified = false;
        u16 bookmark_address = (u16)selected_address;

        if (!bookmark_modified && selected_address >= 0)
            snprintf(address_bookmark, 5, "%04X", bookmark_address);

        ImGui::Text("Name:");
        ImGui::PushItemWidth(200);ImGui::SetItemDefaultFocus();
        ImGui::InputText("##name", name_bookmark, IM_ARRAYSIZE(name_bookmark));

        ImGui::Text("Address:");
        ImGui::PushItemWidth(50);
        if (ImGui::InputTextWithHint("##bookaddr", "XXXX", address_bookmark, IM_ARRAYSIZE(address_bookmark), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            bookmark_modified = true;
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(90, 0)))
        {
            u16 bookmark_address = 0;
            if (parse_hex_string(address_bookmark, strlen(address_bookmark), &bookmark_address))
            {
                if (strlen(name_bookmark) == 0)
                {
                    Memory* memory = emu_get_core()->GetMemory();
                    GS_Disassembler_Record* record = memory->GetDisassemblerRecord(bookmark_address);

                    if (IsValidPointer(record) && (record->name[0] != 0))
                    {
                        std::string instr = record->name;
                        size_t pos = instr.find("{}");
                        if (pos != std::string::npos)
                            instr.replace(pos, 2, "");
                        snprintf(name_bookmark, 32, "%s", instr.c_str());
                    }
                    else
                    {
                        snprintf(name_bookmark, 32, "Bookmark_%04X", bookmark_address);
                    }
                }

                DisassemblerBookmark bookmark;
                bookmark.address = bookmark_address;
                snprintf(bookmark.name, 32, "%s", name_bookmark);
                bookmarks.push_back(bookmark);
                ImGui::CloseCurrentPopup();

                address_bookmark[0] = 0;
                name_bookmark[0] = 0;
                bookmark_modified = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0)))
        {
            address_bookmark[0] = 0;
            name_bookmark[0] = 0;
            bookmark_modified = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void add_symbol_popup(void)
{
    if (add_symbol_open)
    {
        ImGui::OpenPopup("Add Symbol");
        add_symbol_open = false;
    }

    if (ImGui::BeginPopupModal("Add Symbol", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char address[8] = "";
        static char name[32] = "";
        static bool symbol_modified = false;

        if (!symbol_modified && selected_address >= 0 && selected_bank >= 0)
            snprintf(address, 8, "%02X:%04X", selected_bank, selected_address);

        ImGui::Text("Name:");
        ImGui::PushItemWidth(200);ImGui::SetItemDefaultFocus();
        ImGui::InputText("##symname", name, IM_ARRAYSIZE(name));

        ImGui::Text("Address:");
        ImGui::PushItemWidth(70);
        if (ImGui::InputTextWithHint("##symaddr", "XX:XXXX", address, IM_ARRAYSIZE(address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsUppercase))
        {
            symbol_modified = true;
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(90, 0)))
        {
            if (strlen(name) != 0 && strlen(address) != 0)
            {
                char symbol[128] = { };
                snprintf(symbol, 128, "%s %s", address, name);
                add_symbol(symbol);

                ImGui::CloseCurrentPopup();
                address[0] = 0;
                name[0] = 0;
                symbol_modified = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0)))
        {
            ImGui::CloseCurrentPopup();
            address[0] = 0;
            name[0] = 0;
            symbol_modified = false;
        }

        ImGui::EndPopup();
    }
}

void gui_debug_window_call_stack(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(140, 122), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(330, 240), ImGuiCond_FirstUseEver);

    ImGui::Begin("Call Stack", &config_debug.show_call_stack);

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    std::stack<Processor::GS_CallStackEntry> temp_stack = *processor->GetDisassemblerCallStack();

    char symbol_text[64] = { };

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("call_stack", 3, flags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.5f);
        ImGui::TableSetupColumn("Return", ImGuiTableColumnFlags_WidthStretch, 0.5f);
        ImGui::TableHeadersRow();

        ImGui::PushFont(gui_default_font);

        int row_index = 0;
        while (!temp_stack.empty())
        {
            ImGui::TableNextRow();

            Processor::GS_CallStackEntry entry = temp_stack.top();
            temp_stack.pop();

            symbol_text[0] = 0;

            GS_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.dest, entry.bank);

            if (IsValidPointer(record) && (record->name[0] != 0))
            {
                DebugSymbol* symbol = fixed_symbols[entry.bank][entry.dest];

                if (!IsValidPointer(symbol))
                    symbol = dynamic_symbols[entry.bank][entry.dest];

                if (IsValidPointer(symbol))
                    snprintf(symbol_text, sizeof(symbol_text), "%s", symbol->text);
                else if (record->auto_symbol[0] != 0)
                    snprintf(symbol_text, sizeof(symbol_text), "%s", record->auto_symbol);
            }

            ImGui::TableNextColumn();
            char selectable_id[16];
            snprintf(selectable_id, sizeof(selectable_id), "##cs%d", row_index);
            if (ImGui::Selectable(selectable_id, false, ImGuiSelectableFlags_SpanAllColumns))
            {
                request_goto_address(entry.dest);
            }

            ImGui::PopFont();
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::Selectable("Add Breakpoint"))
                {
                    if (!emu_get_core()->GetProcessor()->IsBreakpoint(Processor::GS_BREAKPOINT_TYPE_ROMRAM, entry.dest))
                        emu_get_core()->GetProcessor()->AddBreakpoint(entry.dest);
                }

                ImGui::EndPopup();
            }
            ImGui::PushFont(gui_default_font);

            ImGui::SameLine(0, 0);
            ImGui::TextColored(cyan, "%04X", entry.dest);
            ImGui::SameLine();
            ImGui::TextColored(green, " %s", symbol_text);

            ImGui::TableNextColumn();
            ImGui::TextColored(cyan, "%04X", entry.src);

            ImGui::TableNextColumn();
            ImGui::TextColored(cyan, "%04X", entry.back);

            row_index++;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(gray, "----- Bottom of Stack");
        ImGui::TableNextColumn();
        ImGui::TextColored(gray, "-----");
        ImGui::TableNextColumn();
        ImGui::TextColored(gray, "-----");

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_symbols(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(356, 370), ImGuiCond_FirstUseEver);

    ImGui::Begin("Symbols", &config_debug.show_symbols);

    static char symbol_filter[64] = "";
    static std::vector<SymbolEntry> sorted_symbols;
    static int last_sort_column = -1;
    static int last_sort_direction = -1;

    bool prev_auto = show_auto_symbols;
    ImGui::Checkbox("Automatic Symbols", &show_auto_symbols);
    if (show_auto_symbols != prev_auto)
        symbols_dirty = true;
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##symbol_filter", "Filter...", symbol_filter, IM_ARRAYSIZE(symbol_filter)))
        symbols_dirty = true;
    ImGui::PopItemWidth();

    ImGui::Separator();

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;

    if (ImGui::BeginTable("symbols_table", 4, flags))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Bank", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 44.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 66.0f);
        ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 44.0f);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        {
            if (sort_specs->SpecsDirty || symbols_dirty)
            {
                sort_specs->SpecsDirty = false;
                symbols_dirty = true;
            }

            if (sort_specs->SpecsCount > 0)
            {
                last_sort_column = sort_specs->Specs[0].ColumnIndex;
                last_sort_direction = sort_specs->Specs[0].SortDirection;
            }
        }

        if (symbols_dirty)
        {
            symbols_dirty = false;
            sorted_symbols.clear();

            for (size_t i = 0; i < fixed_symbol_list.size(); i++)
            {
                SymbolEntry& e = fixed_symbol_list[i];

                if (symbol_filter[0] != 0)
                {
                    char addr_str[8];
                    snprintf(addr_str, sizeof(addr_str), "%04X", e.symbol->address);

                    char filter_upper[64];
                    char text_upper[64];
                    for (int j = 0; j < 63 && symbol_filter[j]; j++) { filter_upper[j] = toupper(symbol_filter[j]); filter_upper[j + 1] = 0; }
                    for (int j = 0; j < 63 && e.symbol->text[j]; j++) { text_upper[j] = toupper(e.symbol->text[j]); text_upper[j + 1] = 0; }

                    if (strstr(text_upper, filter_upper) == NULL && strstr(addr_str, filter_upper) == NULL)
                        continue;
                }

                sorted_symbols.push_back(e);
            }

            if (show_auto_symbols)
            {
                for (size_t i = 0; i < dynamic_symbol_list.size(); i++)
                {
                    SymbolEntry& e = dynamic_symbol_list[i];

                    if (IsValidPointer(fixed_symbols[e.bank][e.symbol->address]))
                        continue;

                    if (symbol_filter[0] != 0)
                    {
                        char addr_str[8];
                        snprintf(addr_str, sizeof(addr_str), "%04X", e.symbol->address);

                        char filter_upper[64];
                        char text_upper[64];
                        for (int j = 0; j < 63 && symbol_filter[j]; j++) { filter_upper[j] = toupper(symbol_filter[j]); filter_upper[j + 1] = 0; }
                        for (int j = 0; j < 63 && e.symbol->text[j]; j++) { text_upper[j] = toupper(e.symbol->text[j]); text_upper[j + 1] = 0; }

                        if (strstr(text_upper, filter_upper) == NULL && strstr(addr_str, filter_upper) == NULL)
                            continue;
                    }

                    sorted_symbols.push_back(e);
                }
            }

            if (last_sort_column >= 0)
            {
                bool ascending = (last_sort_direction == ImGuiSortDirection_Ascending);

                if (last_sort_column == 0)
                {
                    std::sort(sorted_symbols.begin(), sorted_symbols.end(), ascending ? symbol_sort_address_asc : symbol_sort_address_desc);
                }
                else if (last_sort_column == 1)
                {
                    std::sort(sorted_symbols.begin(), sorted_symbols.end(), ascending ? symbol_sort_addr_only_asc : symbol_sort_addr_only_desc);
                }
                else if (last_sort_column == 2)
                {
                    std::sort(sorted_symbols.begin(), sorted_symbols.end(), ascending ? symbol_sort_name_asc : symbol_sort_name_desc);
                }
            }
        }

        ImGui::PushFont(gui_default_font);

        ImGuiListClipper clipper;
        clipper.Begin((int)sorted_symbols.size());
        while (clipper.Step())
        {
            for (int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; idx++)
            {
                DebugSymbol* symbol = sorted_symbols[idx].symbol;
                bool is_manual = sorted_symbols[idx].is_manual;
                int b = sorted_symbols[idx].bank;

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                char selectable_id[16];
                snprintf(selectable_id, sizeof(selectable_id), "##sym%d", (int)idx);
                if (ImGui::Selectable(selectable_id, false, ImGuiSelectableFlags_SpanAllColumns))
                {
                    request_goto_address(symbol->address);
                }

                ImGui::PopFont();
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::Selectable("Add Breakpoint"))
                    {
                        if (!emu_get_core()->GetProcessor()->IsBreakpoint(Processor::GS_BREAKPOINT_TYPE_ROMRAM, symbol->address))
                            emu_get_core()->GetProcessor()->AddBreakpoint(symbol->address);
                    }

                    if (is_manual)
                    {
                        if (ImGui::Selectable("Remove Symbol"))
                        {
                            gui_debug_remove_symbol(b, symbol->address);
                        }
                    }

                    ImGui::EndPopup();
                }
                ImGui::PushFont(gui_default_font);

                ImGui::SameLine(0, 0);
                ImGui::TextColored(violet, " %02X", b);

                ImGui::TableNextColumn();
                ImGui::TextColored(cyan, " %04X", symbol->address);

                ImGui::TableNextColumn();
                ImGui::TextColored(is_manual ? green : yellow, "%s", symbol->text);

                ImGui::TableNextColumn();
                if (is_manual)
                    ImGui::TextColored(orange, "Manual");
                else
                    ImGui::TextColored(brown, "Auto");
            }
        }

        ImGui::PopFont();

        ImGui::EndTable();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_add_symbol(const char* symbol_str)
{
    add_symbol(symbol_str);
}

void gui_debug_remove_symbol(u8 bank, u16 address)
{
    DebugSymbol* symbol = fixed_symbols[bank][address];
    if (IsValidPointer(symbol))
    {
        for (size_t i = 0; i < fixed_symbol_list.size(); i++)
        {
            if (fixed_symbol_list[i].symbol == symbol)
            {
                fixed_symbol_list.erase(fixed_symbol_list.begin() + i);
                break;
            }
        }
        delete symbol;
        fixed_symbols[bank][address] = NULL;
        symbols_dirty = true;
    }
}

void gui_debug_add_disassembler_bookmark(u16 address, const char* name)
{
    DisassemblerBookmark bookmark;
    bookmark.address = address;

    if (name && strlen(name) > 0)
    {
        snprintf(bookmark.name, 32, "%s", name);
    }
    else
    {
        // Auto-generate name from instruction
        Memory* memory = emu_get_core()->GetMemory();
        GS_Disassembler_Record* record = memory->GetDisassemblerRecord(address);

        if (IsValidPointer(record) && (record->name[0] != 0))
        {
            std::string instr = record->name;
            size_t pos = instr.find("{}");
            if (pos != std::string::npos)
                instr.replace(pos, 2, "");
            snprintf(bookmark.name, 32, "%s", instr.c_str());
        }
        else
        {
            snprintf(bookmark.name, 32, "Bookmark_%04X", address);
        }
    }

    bookmarks.push_back(bookmark);
}

void gui_debug_remove_disassembler_bookmark(u16 address)
{
    for (std::vector<DisassemblerBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        if (it->address == address)
        {
            bookmarks.erase(it);
            break;
        }
    }
}

int gui_debug_get_disassembler_bookmarks(void** bookmarks_ptr)
{
    *bookmarks_ptr = (void*)&bookmarks;
    return (int)bookmarks.size();
}

void gui_debug_reset_disassembler_bookmarks(void)
{
    bookmarks.clear();
}

int gui_debug_get_symbols(void** symbols_ptr)
{
    *symbols_ptr = (void*)fixed_symbols;
    return 0x100; // 256 banks
}

static void save_full_disassembler(FILE* file)
{
    Memory* memory = emu_get_core()->GetMemory();
    GS_Disassembler_Record** records = memory->GetAllDisassemblerRecords();

    for (int i = 0; i < 0x200000; i++)
    {
        GS_Disassembler_Record* record = records[i];

        if (IsValidPointer(record) && (record->name[0] != 0))
        {
            if (record->subroutine || record->irq)
                fprintf(file, "\n");

            char name[64];
            strcpy(name, record->name);
            RemoveColorFromString(name);

            int len = (int)strlen(name);
            char spaces[32];
            int offset = 28 - len;
            if (offset < 0)
                offset = 0;
            for (int i = 0; i < offset; i++)
                spaces[i] = ' ';
            spaces[offset] = 0;

            fprintf(file, "%06X-%02X:    %s%s;%s\n", i, record->bank, name, spaces, record->bytes);

            if (is_return_instruction(record))
                fprintf(file, "\n");
        }
    }
}

static void save_current_disassembler(FILE* file)
{
    int total_lines = (int)disassembler_lines.size();

    for (int i = 0; i < total_lines; i++)
    {
        DisassemblerLine line = disassembler_lines[i];

        if (line.symbol)
        {
            fprintf(file, "%s:\n", line.symbol->text);
            continue;
        }

        fprintf(file, "  ");

        if (config_debug.dis_show_segment)
            fprintf(file, "%s ", line.record->segment);
        if (config_debug.dis_show_bank)
            fprintf(file, "%02X ", line.record->bank);

        fprintf(file, " %04X ", line.address);

        if (config_debug.dis_replace_symbols)
        {
            replace_symbols(&line, "", "", "", "");
        }

        if (config_debug.dis_replace_labels)
        {
            replace_labels(&line, "", "");
        }

        char instr[64];
        snprintf(instr, sizeof(instr), "%s", line.name_enhanced);
        RemoveColorFromString(instr);

        fprintf(file, "   %s ", instr);

        if (config_debug.dis_show_mem)
        {
            int len = (int)strlen(instr);
            char spaces[39];
            int offset = 38 - len;
            if (offset < 0)
                offset = 0;
            for (int i = 0; (i < offset) && (i < 38); i++)
                spaces[i] = ' ';
            spaces[offset] = 0;

            fprintf(file, "%s;%s", spaces, line.record->bytes);
        }

        fprintf(file, "\n");

        if (is_return_instruction(line.record))
        {
            fprintf(file, "\n\n");
        }
    }
}

static bool symbol_sort_address_asc(const SymbolEntry& a, const SymbolEntry& b)
{
    if (a.bank != b.bank)
        return a.bank < b.bank;
    return a.symbol->address < b.symbol->address;
}

static bool symbol_sort_address_desc(const SymbolEntry& a, const SymbolEntry& b)
{
    if (a.bank != b.bank)
        return a.bank > b.bank;
    return a.symbol->address > b.symbol->address;
}

static bool symbol_sort_addr_only_asc(const SymbolEntry& a, const SymbolEntry& b)
{
    if (a.symbol->address != b.symbol->address)
        return a.symbol->address < b.symbol->address;
    return a.bank < b.bank;
}

static bool symbol_sort_addr_only_desc(const SymbolEntry& a, const SymbolEntry& b)
{
    if (a.symbol->address != b.symbol->address)
        return a.symbol->address > b.symbol->address;
    return a.bank > b.bank;
}

static bool symbol_sort_name_asc(const SymbolEntry& a, const SymbolEntry& b)
{
    return strcmp(a.symbol->text, b.symbol->text) < 0;
}

static bool symbol_sort_name_desc(const SymbolEntry& a, const SymbolEntry& b)
{
    return strcmp(a.symbol->text, b.symbol->text) > 0;
}
