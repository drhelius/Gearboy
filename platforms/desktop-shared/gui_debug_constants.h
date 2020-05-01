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

#ifndef GUI_DEBUG_CONSTANTS_H
#define	GUI_DEBUG_CONSTANTS_H

static const int gui_debug_symbols_count = 14;

static const char* gui_debug_symbols[gui_debug_symbols_count] = {
    "00:0000 RST_00",
    "00:0008 RST_08",
    "00:0010 RST_10",
    "00:0018 RST_18",
    "00:0020 RST_20",
    "00:0028 RST_28",
    "00:0030 RST_30",
    "00:0038 RST_38",
    "00:0040 VBlankInterrupt",
    "00:0048 LCDCInterrupt",
    "00:0050 TimerOverflowInterrupt",
    "00:0058 SerialTransferCompleteInterrupt",
    "00:0060 JoypadTransitionInterrupt",
    "00:0100 Boot"
};

#endif	/* GUI_DEBUG_CONSTANTS_H */