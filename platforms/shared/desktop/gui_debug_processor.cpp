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
#include "gui_debug_widgets.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

enum SM83RegId
{
    SM83RegId_A = 0,
    SM83RegId_F,
    SM83RegId_B,
    SM83RegId_C,
    SM83RegId_D,
    SM83RegId_E,
    SM83RegId_H,
    SM83RegId_L,
    SM83RegId_SP,
    SM83RegId_PC,
    SM83RegId_IME
};

static void SM83WriteCallback8(u16 reg_id, u8 value, void* user_data)
{
    GearboyCore* core = (GearboyCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    switch (reg_id)
    {
        case SM83RegId_A:
            proc_state->AF->SetHigh(value);
            break;
        case SM83RegId_F:
            proc_state->AF->SetLow(value & 0xF0);
            break;
        case SM83RegId_B:
            proc_state->BC->SetHigh(value);
            break;
        case SM83RegId_C:
            proc_state->BC->SetLow(value);
            break;
        case SM83RegId_D:
            proc_state->DE->SetHigh(value);
            break;
        case SM83RegId_E:
            proc_state->DE->SetLow(value);
            break;
        case SM83RegId_H:
            proc_state->HL->SetHigh(value);
            break;
        case SM83RegId_L:
            proc_state->HL->SetLow(value);
            break;
    }
}

static void SM83WriteCallback1(u16 reg_id, u8 bit_index, bool value, void* user_data)
{
    GearboyCore* core = (GearboyCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    if (reg_id == SM83RegId_F)
    {
        u8 f = proc_state->AF->GetLow();
        if (value)
            f |= (1 << bit_index);
        else
            f &= ~(1 << bit_index);
        proc_state->AF->SetLow(f & 0xF0);
    }
    else if (reg_id == SM83RegId_IME)
    {
        *proc_state->IME = value;
    }
}

static void SM83WriteCallback16(u16 reg_id, u16 value, void* user_data)
{
    GearboyCore* core = (GearboyCore*)user_data;
    Processor::ProcessorState* proc_state = core->GetProcessor()->GetState();

    switch (reg_id)
    {
        case SM83RegId_SP: proc_state->SP->SetValue(value); break;
        case SM83RegId_PC: proc_state->PC->SetValue(value); break;
    }
}

void gui_debug_window_processor(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(3, 26), ImGuiCond_FirstUseEver);

    ImGui::Begin("Processor", &config_debug.show_processor, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::PushFont(gui_default_font);

    GearboyCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    Memory* memory = core->GetMemory();

    if (ImGui::BeginTable("sm83", 1, ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableNextColumn();
        u8 f = proc_state->AF->GetLow();
        ImGui::Text("     ");
        ImGui::SameLine(0, 0); ImGui::TextColored(orange, "Z");
        ImGui::SameLine(); ImGui::TextColored(orange, "N");
        ImGui::SameLine(); ImGui::TextColored(orange, "H");
        ImGui::SameLine(); ImGui::TextColored(orange, "C");
        ImGui::Text("     ");
        ImGui::SameLine(0, 0);
        EditableRegister1(SM83RegId_F, 7, (f >> 7) & 1, SM83WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(SM83RegId_F, 6, (f >> 6) & 1, SM83WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(SM83RegId_F, 5, (f >> 5) & 1, SM83WriteCallback1, core);
        ImGui::SameLine(); EditableRegister1(SM83RegId_F, 4, (f >> 4) & 1, SM83WriteCallback1, core);

        ImGui::TableNextColumn();
        ImGui::TextColored(yellow, "    PC"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, SM83RegId_PC, proc_state->PC->GetValue(), SM83WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->PC->GetHigh()), BYTE_TO_BINARY(proc_state->PC->GetLow()));

        ImGui::TableNextColumn();
        ImGui::TextColored(yellow, "    SP"); ImGui::SameLine();
        ImGui::Text(" "); ImGui::SameLine(0, 0);
        EditableRegister16(NULL, NULL, SM83RegId_SP, proc_state->SP->GetValue(), SM83WriteCallback16, core, EditableRegisterFlags_None);
        ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->SP->GetHigh()), BYTE_TO_BINARY(proc_state->SP->GetLow()));

        ImGui::TableNextColumn();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 2.0f));

        if (ImGui::BeginTable("regs", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " A"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, SM83RegId_A, proc_state->AF->GetHigh(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_F, proc_state->AF->GetLow(), SM83WriteCallback8, core, EditableRegisterFlags_None);
            ImGui::TextColored(gray, BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::TextColored(cyan, "Hex: $%02X", proc_state->AF->GetLow());
                ImGui::TextColored(cyan, "Dec: %u (%d)", proc_state->AF->GetLow(), (s8)proc_state->AF->GetLow());
                ImGui::TextColored(cyan, "Bin: " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));
                ImGui::EndTooltip();
            }

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::TextColored(cyan, " B"); ImGui::SameLine();
            ImGui::Text("  "); ImGui::SameLine(0, 0);
            EditableRegister8(NULL, NULL, SM83RegId_B, proc_state->BC->GetHigh(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_C, proc_state->BC->GetLow(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_D, proc_state->DE->GetHigh(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_E, proc_state->DE->GetLow(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_H, proc_state->HL->GetHigh(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            EditableRegister8(NULL, NULL, SM83RegId_L, proc_state->HL->GetLow(), SM83WriteCallback8, core, EditableRegisterFlags_None);
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
            ImGui::TextColored(*proc_state->IME ? green : gray, "   IME");
            ImGui::TableNextColumn();
            ImGui::TextColored(*proc_state->Halt ? green : gray, "  HALT");

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();

        ImGui::TableNextColumn();
        ImGui::TextColored(processor->CGBSpeed() ? green : gray, "   DOUBLE SPEED");
        ImGui::TableNextColumn();
        ImGui::TextColored(memory->IsBootromRegistryEnabled() ? green : gray, "     BOOTROM");

        ImGui::EndTable();
    }

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}
