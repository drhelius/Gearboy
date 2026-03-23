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

#define GUI_DEBUG_IO_IMPORT
#include "gui_debug_io.h"

#include "imgui.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

void gui_debug_window_io(void)
{
    ImGui::SetNextWindowPos(ImVec2(121, 164), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("IO Map", &config_debug.show_io);

    ImGui::PushFont(gui_default_font);

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();

    ImGui::Columns(2, "iomap");

    ImGui::TextColored(magenta, "INTERRUPTS:");

    ImGui::TextColored(cyan, " $FFFF"); ImGui::SameLine();
    ImGui::TextColored(violet, "IE  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFFFF), BYTE_TO_BINARY(memory->Retrieve(0xFFFF)));

    ImGui::TextColored(cyan, " $FF0F"); ImGui::SameLine();
    ImGui::TextColored(violet, "IF  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF0F), BYTE_TO_BINARY(memory->Retrieve(0xFF0F)));

    ImGui::TextColored(violet, " VBLNK  "); ImGui::SameLine();
    IsSetBit(memory->Retrieve(0xFF0F), 0) && IsSetBit(memory->Retrieve(0xFFFF), 0) ? ImGui::TextColored(green, "ON  ") : ImGui::TextColored(gray, "OFF "); ImGui::SameLine();
    ImGui::TextColored(violet, "IF:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFF0F), 0)); ImGui::SameLine();
    ImGui::TextColored(violet, "  IE:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFFFF), 0));

    ImGui::TextColored(violet, " STAT   "); ImGui::SameLine();
    IsSetBit(memory->Retrieve(0xFF0F), 1) && IsSetBit(memory->Retrieve(0xFFFF), 1) ? ImGui::TextColored(green, "ON  ") : ImGui::TextColored(gray, "OFF "); ImGui::SameLine();
    ImGui::TextColored(violet, "IF:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFF0F), 1)); ImGui::SameLine();
    ImGui::TextColored(violet, "  IE:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFFFF), 1));

    ImGui::TextColored(violet, " TIMER  "); ImGui::SameLine();
    IsSetBit(memory->Retrieve(0xFF0F), 2) && IsSetBit(memory->Retrieve(0xFFFF), 2) ? ImGui::TextColored(green, "ON  ") : ImGui::TextColored(gray, "OFF "); ImGui::SameLine();
    ImGui::TextColored(violet, "IF:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFF0F), 2)); ImGui::SameLine();
    ImGui::TextColored(violet, "  IE:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFFFF), 2));

    ImGui::TextColored(violet, " SERIAL "); ImGui::SameLine();
    IsSetBit(memory->Retrieve(0xFF0F), 3) && IsSetBit(memory->Retrieve(0xFFFF), 3) ? ImGui::TextColored(green, "ON  ") : ImGui::TextColored(gray, "OFF "); ImGui::SameLine();
    ImGui::TextColored(violet, "IF:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFF0F), 3)); ImGui::SameLine();
    ImGui::TextColored(violet, "  IE:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFFFF), 3));

    ImGui::TextColored(violet, " JOYPAD "); ImGui::SameLine();
    IsSetBit(memory->Retrieve(0xFF0F), 4) && IsSetBit(memory->Retrieve(0xFFFF), 4) ? ImGui::TextColored(green, "ON  ") : ImGui::TextColored(gray, "OFF "); ImGui::SameLine();
    ImGui::TextColored(violet, "IF:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFF0F), 4)); ImGui::SameLine();
    ImGui::TextColored(violet, "  IE:"); ImGui::SameLine();
    ImGui::Text("%d", IsSetBit(memory->Retrieve(0xFFFF), 4));

    ImGui::TextColored(magenta, "GBC:");

    ImGui::TextColored(cyan, " $FF4D"); ImGui::SameLine();
    ImGui::TextColored(violet, "KEY1"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF4D), BYTE_TO_BINARY(memory->Retrieve(0xFF4D)));

    ImGui::TextColored(cyan, " $FF70"); ImGui::SameLine();
    ImGui::TextColored(violet, "SVBK"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF70), BYTE_TO_BINARY(memory->Retrieve(0xFF70)));

    ImGui::TextColored(magenta, "GBC LCD:");

    ImGui::TextColored(cyan, " $FF68"); ImGui::SameLine();
    ImGui::TextColored(violet, "BCPS"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF68), BYTE_TO_BINARY(memory->Retrieve(0xFF68)));

    ImGui::TextColored(cyan, " $FF69"); ImGui::SameLine();
    ImGui::TextColored(violet, "BCPD"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF69), BYTE_TO_BINARY(memory->Retrieve(0xFF69)));

    ImGui::TextColored(cyan, " $FF6A"); ImGui::SameLine();
    ImGui::TextColored(violet, "OCPS"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF6A), BYTE_TO_BINARY(memory->Retrieve(0xFF6A)));

    ImGui::TextColored(cyan, " $FF6B"); ImGui::SameLine();
    ImGui::TextColored(violet, "OCPD"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF6B), BYTE_TO_BINARY(memory->Retrieve(0xFF6B)));

    ImGui::TextColored(cyan, " $FF4F"); ImGui::SameLine();
    ImGui::TextColored(violet, "VBK "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF4F), BYTE_TO_BINARY(memory->Retrieve(0xFF4F)));

    ImGui::TextColored(magenta, "GBC HDMA:");

    ImGui::TextColored(cyan, " $FF51:$FF52"); ImGui::SameLine();
    ImGui::TextColored(violet, "SOURCE "); ImGui::SameLine();
    ImGui::Text("$%04X", (memory->Retrieve(0xFF51) << 8) | memory->Retrieve(0xFF52));

    ImGui::TextColored(cyan, " $FF53:$FF54"); ImGui::SameLine();
    ImGui::TextColored(violet, "DEST   "); ImGui::SameLine();
    ImGui::Text("$%04X", (memory->Retrieve(0xFF53) << 8) | memory->Retrieve(0xFF54));

    ImGui::TextColored(cyan, " $FF55"); ImGui::SameLine();
    ImGui::TextColored(violet, "LEN "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF55), BYTE_TO_BINARY(memory->Retrieve(0xFF55)));

    ImGui::TextColored(magenta, "GBC INFRARED:");

    ImGui::TextColored(cyan, " $FF56"); ImGui::SameLine();
    ImGui::TextColored(violet, "RP  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF56), BYTE_TO_BINARY(memory->Retrieve(0xFF56)));

    ImGui::NextColumn();

    ImGui::TextColored(magenta, "LCD:");

    ImGui::TextColored(cyan, " $FF40"); ImGui::SameLine();
    ImGui::TextColored(violet, "LCDC"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF40), BYTE_TO_BINARY(memory->Retrieve(0xFF40)));

    ImGui::TextColored(cyan, " $FF41"); ImGui::SameLine();
    ImGui::TextColored(violet, "STAT"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF41), BYTE_TO_BINARY(memory->Retrieve(0xFF41)));

    ImGui::TextColored(cyan, " $FF42"); ImGui::SameLine();
    ImGui::TextColored(violet, "SCY "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF42), BYTE_TO_BINARY(memory->Retrieve(0xFF42)));

    ImGui::TextColored(cyan, " $FF43"); ImGui::SameLine();
    ImGui::TextColored(violet, "SCX "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF43), BYTE_TO_BINARY(memory->Retrieve(0xFF43)));

    ImGui::TextColored(cyan, " $FF44"); ImGui::SameLine();
    ImGui::TextColored(violet, "LY  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF44), BYTE_TO_BINARY(memory->Retrieve(0xFF44)));

    ImGui::TextColored(cyan, " $FF45"); ImGui::SameLine();
    ImGui::TextColored(violet, "LYC "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF45), BYTE_TO_BINARY(memory->Retrieve(0xFF45)));

    ImGui::TextColored(cyan, " $FF46"); ImGui::SameLine();
    ImGui::TextColored(violet, "DMA "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF46), BYTE_TO_BINARY(memory->Retrieve(0xFF46)));

    ImGui::TextColored(cyan, " $FF47"); ImGui::SameLine();
    ImGui::TextColored(violet, "BGP "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF47), BYTE_TO_BINARY(memory->Retrieve(0xFF47)));

    ImGui::TextColored(cyan, " $FF48"); ImGui::SameLine();
    ImGui::TextColored(violet, "OBP0"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF48), BYTE_TO_BINARY(memory->Retrieve(0xFF48)));

    ImGui::TextColored(cyan, " $FF49"); ImGui::SameLine();
    ImGui::TextColored(violet, "OBP1"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF49), BYTE_TO_BINARY(memory->Retrieve(0xFF49)));

    ImGui::TextColored(cyan, " $FF4A"); ImGui::SameLine();
    ImGui::TextColored(violet, "WY  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF4A), BYTE_TO_BINARY(memory->Retrieve(0xFF4A)));

    ImGui::TextColored(cyan, " $FF4B"); ImGui::SameLine();
    ImGui::TextColored(violet, "WX  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF4B), BYTE_TO_BINARY(memory->Retrieve(0xFF4B)));

    ImGui::TextColored(magenta, "TIMER:");

    ImGui::TextColored(cyan, " $FF04"); ImGui::SameLine();
    ImGui::TextColored(violet, "DIV "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF04), BYTE_TO_BINARY(memory->Retrieve(0xFF04)));

    ImGui::TextColored(cyan, " $FF05"); ImGui::SameLine();
    ImGui::TextColored(violet, "TIMA"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF05), BYTE_TO_BINARY(memory->Retrieve(0xFF05)));

    ImGui::TextColored(cyan, " $FF06"); ImGui::SameLine();
    ImGui::TextColored(violet, "TMA "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF06), BYTE_TO_BINARY(memory->Retrieve(0xFF06)));

    ImGui::TextColored(cyan, " $FF07"); ImGui::SameLine();
    ImGui::TextColored(violet, "TAC "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF07), BYTE_TO_BINARY(memory->Retrieve(0xFF07)));

    ImGui::TextColored(magenta, "INPUT:");

    ImGui::TextColored(cyan, " $FF00"); ImGui::SameLine();
    ImGui::TextColored(violet, "JOYP"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF00), BYTE_TO_BINARY(memory->Retrieve(0xFF00)));

    ImGui::TextColored(magenta, "SERIAL:");

    ImGui::TextColored(cyan, " $FF01"); ImGui::SameLine();
    ImGui::TextColored(violet, "SB  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF01), BYTE_TO_BINARY(memory->Retrieve(0xFF01)));

    ImGui::TextColored(cyan, " $FF02"); ImGui::SameLine();
    ImGui::TextColored(violet, "SC  "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")", memory->Retrieve(0xFF02), BYTE_TO_BINARY(memory->Retrieve(0xFF02)));

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::End();
}
