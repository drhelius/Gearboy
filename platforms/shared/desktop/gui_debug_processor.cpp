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

#define GUI_DEBUG_PROCESSOR_IMPORT
#include "gui_debug_processor.h"

#include "imgui.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui_debug_memory.h"
#include "gui_debug_widgets.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

enum Z80RegId
{
    Z80RegId_A = 0,
    Z80RegId_F,
    Z80RegId_B,
    Z80RegId_C,
    Z80RegId_D,
    Z80RegId_E,
    Z80RegId_H,
    Z80RegId_L,
    Z80RegId_A2,
    Z80RegId_F2,
    Z80RegId_B2,
    Z80RegId_C2,
    Z80RegId_D2,
    Z80RegId_E2,
    Z80RegId_H2,
    Z80RegId_L2,
    Z80RegId_I,
    Z80RegId_R,
    Z80RegId_IX,
    Z80RegId_IY,
    Z80RegId_SP,
    Z80RegId_PC,
    Z80RegId_WZ,
    Z80RegId_IFF1,
    Z80RegId_IFF2
};

static void Z80WriteCallback8(u16 reg_id, u8 value, void* user_data)
{
    GearsystemCore* core = (GearsystemCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    switch (reg_id)
    {
        case Z80RegId_A:  proc_state->AF->SetHigh(value); break;
        case Z80RegId_F:  proc_state->AF->SetLow(value); break;
        case Z80RegId_B:  proc_state->BC->SetHigh(value); break;
        case Z80RegId_C:  proc_state->BC->SetLow(value); break;
        case Z80RegId_D:  proc_state->DE->SetHigh(value); break;
        case Z80RegId_E:  proc_state->DE->SetLow(value); break;
        case Z80RegId_H:  proc_state->HL->SetHigh(value); break;
        case Z80RegId_L:  proc_state->HL->SetLow(value); break;
        case Z80RegId_A2: proc_state->AF2->SetHigh(value); break;
        case Z80RegId_F2: proc_state->AF2->SetLow(value); break;
        case Z80RegId_B2: proc_state->BC2->SetHigh(value); break;
        case Z80RegId_C2: proc_state->BC2->SetLow(value); break;
        case Z80RegId_D2: proc_state->DE2->SetHigh(value); break;
        case Z80RegId_E2: proc_state->DE2->SetLow(value); break;
        case Z80RegId_H2: proc_state->HL2->SetHigh(value); break;
        case Z80RegId_L2: proc_state->HL2->SetLow(value); break;
        case Z80RegId_I:  *proc_state->I = value; break;
        case Z80RegId_R:  *proc_state->R = value; break;
    }
}

static void Z80WriteCallback1(u16 reg_id, u8 bit_index, bool value, void* user_data)
{
    GearsystemCore* core = (GearsystemCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    if (reg_id == Z80RegId_F)
    {
        u8 f = proc_state->AF->GetLow();
        if (value)
            f |= (1 << bit_index);
        else
            f &= ~(1 << bit_index);
        proc_state->AF->SetLow(f);
    }
    else if (reg_id == Z80RegId_F2)
    {
        u8 f2 = proc_state->AF2->GetLow();
        if (value)
            f2 |= (1 << bit_index);
        else
            f2 &= ~(1 << bit_index);
        proc_state->AF2->SetLow(f2);
    }
    else if (reg_id == Z80RegId_IFF1)
    {
        *proc_state->IFF1 = value;
    }
    else if (reg_id == Z80RegId_IFF2)
    {
        *proc_state->IFF2 = value;
    }
}

static void Z80WriteCallback16(u16 reg_id, u16 value, void* user_data)
{
    GearsystemCore* core = (GearsystemCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    switch (reg_id)
    {
        case Z80RegId_IX: proc_state->IX->SetValue(value); break;
        case Z80RegId_IY: proc_state->IY->SetValue(value); break;
        case Z80RegId_WZ: proc_state->WZ->SetValue(value); break;
        case Z80RegId_SP: proc_state->SP->SetValue(value); break;
        case Z80RegId_PC: proc_state->PC->SetValue(value); break;
    }
}

void gui_debug_window_processor(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(3, 26), ImGuiCond_FirstUseEver);

    ImGui::Begin("Z80", &config_debug.show_processor, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::PushFont(gui_default_font);

    GearsystemCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();

    if (ImGui::BeginTable("z80", 1, ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableNextColumn();
        u8 f = proc_state->AF->GetLow();
        ImGui::Text(" ");
        ImGui::SameLine(0, 0); ImGui::TextColored(orange, "S");
        ImGui::SameLine(); ImGui::TextColored(orange, "Z");
        ImGui::SameLine(); ImGui::TextColored(orange, "Y");
        ImGui::SameLine(); ImGui::TextColored(orange, "H");
        ImGui::SameLine(); ImGui::TextColored(orange, "X");
        ImGui::SameLine(); ImGui::TextColored(orange, "P");
        ImGui::SameLine(); ImGui::TextColored(orange, "N");
        ImGui::SameLine(); ImGui::TextColored(orange, "C");
        ImGui::Text(" ");
        ImGui::SameLine(0, 0);
        EditableRegister1(Z80RegId_F, 7, (f >> 7) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 6, (f >> 6) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 5, (f >> 5) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 4, (f >> 4) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 3, (f >> 3) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 2, (f >> 2) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 1, (f >> 1) & 1, Z80WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(Z80RegId_F, 0, f & 1, Z80WriteCallback1, core);

        ImGui::TableNextColumn();
        ImGui::TextColored(yellow, "    PC"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, Z80RegId_PC, proc_state->PC->GetValue(), Z80WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->PC->GetHigh()), BYTE_TO_BINARY(proc_state->PC->GetLow()));

        ImGui::TableNextColumn();
        ImGui::TextColored(yellow, "    SP"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, Z80RegId_SP, proc_state->SP->GetValue(), Z80WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->SP->GetHigh()), BYTE_TO_BINARY(proc_state->SP->GetLow()));

        ImGui::TableNextColumn();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 2.0f));

        if (ImGui::BeginTable("regs", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " A"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_A, proc_state->AF->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->AF->GetHigh());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->AF->GetHigh(), (s8)proc_state->AF->GetHigh());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetHigh()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->AF->GetHigh() >= 32 && proc_state->AF->GetHigh() < 127) ? proc_state->AF->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " F"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_F, proc_state->AF->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->AF->GetLow());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->AF->GetLow(), (s8)proc_state->AF->GetLow());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->AF->GetLow() >= 32 && proc_state->AF->GetLow() < 127) ? proc_state->AF->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " B"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_B, proc_state->BC->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->BC->GetHigh());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->BC->GetHigh(), (s8)proc_state->BC->GetHigh());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetHigh()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->BC->GetHigh() >= 32 && proc_state->BC->GetHigh() < 127) ? proc_state->BC->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " C"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_C, proc_state->BC->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->BC->GetLow());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->BC->GetLow(), (s8)proc_state->BC->GetLow());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetLow()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->BC->GetLow() >= 32 && proc_state->BC->GetLow() < 127) ? proc_state->BC->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " D"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_D, proc_state->DE->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->DE->GetHigh());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->DE->GetHigh(), (s8)proc_state->DE->GetHigh());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetHigh()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->DE->GetHigh() >= 32 && proc_state->DE->GetHigh() < 127) ? proc_state->DE->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " E"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_E, proc_state->DE->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->DE->GetLow());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->DE->GetLow(), (s8)proc_state->DE->GetLow());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetLow()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->DE->GetLow() >= 32 && proc_state->DE->GetLow() < 127) ? proc_state->DE->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " H"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_H, proc_state->HL->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->HL->GetHigh());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->HL->GetHigh(), (s8)proc_state->HL->GetHigh());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetHigh()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->HL->GetHigh() >= 32 && proc_state->HL->GetHigh() < 127) ? proc_state->HL->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " L"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_L, proc_state->HL->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->HL->GetLow());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->HL->GetLow(), (s8)proc_state->HL->GetLow());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetLow()));
                ImGui::TextColored(cyan, "Ascii: %c", (proc_state->HL->GetLow() >= 32 && proc_state->HL->GetLow() < 127) ? proc_state->HL->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " A'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_A2, proc_state->AF2->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->AF2->GetHigh());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->AF2->GetHigh(), (s8)proc_state->AF2->GetHigh());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetHigh()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->AF2->GetHigh() >= 32 && proc_state->AF2->GetHigh() < 127) ? proc_state->AF2->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " F'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_F2, proc_state->AF2->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->AF2->GetLow());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->AF2->GetLow(), (s8)proc_state->AF2->GetLow());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetLow()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->AF2->GetLow() >= 32 && proc_state->AF2->GetLow() < 127) ? proc_state->AF2->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " B'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_B2, proc_state->BC2->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->BC2->GetHigh());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->BC2->GetHigh(), (s8)proc_state->BC2->GetHigh());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetHigh()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->BC2->GetHigh() >= 32 && proc_state->BC2->GetHigh() < 127) ? proc_state->BC2->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " C'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_C2, proc_state->BC2->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->BC2->GetLow());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->BC2->GetLow(), (s8)proc_state->BC2->GetLow());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetLow()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->BC2->GetLow() >= 32 && proc_state->BC2->GetLow() < 127) ? proc_state->BC2->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " D'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_D2, proc_state->DE2->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->DE2->GetHigh());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->DE2->GetHigh(), (s8)proc_state->DE2->GetHigh());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetHigh()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->DE2->GetHigh() >= 32 && proc_state->DE2->GetHigh() < 127) ? proc_state->DE2->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " E'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_E2, proc_state->DE2->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->DE2->GetLow());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->DE2->GetLow(), (s8)proc_state->DE2->GetLow());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetLow()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->DE2->GetLow() >= 32 && proc_state->DE2->GetLow() < 127) ? proc_state->DE2->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " H'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_H2, proc_state->HL2->GetHigh(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetHigh()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->HL2->GetHigh());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->HL2->GetHigh(), (s8)proc_state->HL2->GetHigh());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetHigh()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->HL2->GetHigh() >= 32 && proc_state->HL2->GetHigh() < 127) ? proc_state->HL2->GetHigh() : '.');
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(violet, " L'"); ImGui::SameLine();
            ImGui::Text(" "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, Z80RegId_L2, proc_state->HL2->GetLow(), Z80WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(violet, "Hex: $%02X", proc_state->HL2->GetLow());
                ImGui::TextColored(violet, "Dec: %u (%d)", proc_state->HL2->GetLow(), (s8)proc_state->HL2->GetLow());
                ImGui::TextColored(violet, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetLow()));
                ImGui::TextColored(violet, "Ascii: %c", (proc_state->HL2->GetLow() >= 32 && proc_state->HL2->GetLow() < 127) ? proc_state->HL2->GetLow() : '.');
                ImGui::EndTooltip();
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::BeginGroup();
        ImGui::TextColored(yellow, "    IX"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, Z80RegId_IX, proc_state->IX->GetValue(), Z80WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->IX->GetHigh()), BYTE_TO_BINARY(proc_state->IX->GetLow()));
        ImGui::EndGroup();
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextColored(green, "Hex: $%04X", proc_state->IX->GetValue());
            ImGui::TextColored(green, "Dec: %u (%d)", proc_state->IX->GetValue(), (s16)proc_state->IX->GetValue());
            ImGui::EndTooltip();
        }

        ImGui::TableNextColumn();
        ImGui::BeginGroup();
        ImGui::TextColored(yellow, "    IY"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, Z80RegId_IY, proc_state->IY->GetValue(), Z80WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->IY->GetHigh()), BYTE_TO_BINARY(proc_state->IY->GetLow()));
        ImGui::EndGroup();
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextColored(yellow, "Hex: $%04X", proc_state->IY->GetValue());
            ImGui::TextColored(yellow, "Dec: %u (%d)", proc_state->IY->GetValue(), (s16)proc_state->IY->GetValue());
            ImGui::EndTooltip();
        }

        ImGui::TableNextColumn();
        ImGui::BeginGroup();
        ImGui::TextColored(yellow, "    WZ"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, Z80RegId_WZ, proc_state->WZ->GetValue(), Z80WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->WZ->GetHigh()), BYTE_TO_BINARY(proc_state->WZ->GetLow()));
        ImGui::EndGroup();
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextColored(yellow, "Hex: $%04X", proc_state->WZ->GetValue());
            ImGui::TextColored(yellow, "Dec: %u (%d)", proc_state->WZ->GetValue(), (s16)proc_state->WZ->GetValue());
            ImGui::EndTooltip();
        }

        ImGui::TableNextColumn();
        ImGui::TextColored(*proc_state->IFF1 ? green : gray, " IFF1"); ImGui::SameLine();
        ImGui::TextColored(*proc_state->IFF2 ? green : gray, " IFF2"); ImGui::SameLine();
        ImGui::TextColored(*proc_state->Halt ? green : gray, " HALT");

        ImGui::TextColored(*proc_state->INT ? green : gray, "    INT"); ImGui::SameLine();
        ImGui::TextColored(*proc_state->NMI ? green : gray, "  NMI");

        ImGui::EndTable();
    }

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}
