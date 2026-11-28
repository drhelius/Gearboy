/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez
 * Copyright (C) 2026  Nolan Baker

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

#ifndef LIBRETRO_PS2_KEYBOARD_H
#define LIBRETRO_PS2_KEYBOARD_H

#include "libretro.h"
#include "../../src/PS2Keyboard.h"
#include <deque>

class GearboyCore;

void libretro_ps2_keyboard_init(retro_environment_t environment_callback);
void libretro_ps2_keyboard_check_variables();
GB_PS2KeyboardMode libretro_ps2_keyboard_get_mode();
bool libretro_ps2_keyboard_load(GearboyCore* core);
void libretro_ps2_keyboard_unload();
void libretro_ps2_keyboard_reset();
void libretro_ps2_keyboard_update_input();

// Keep complete key sequences pending when the emulated serial queue is full.
class LibretroPS2Keyboard
{
public:
    LibretroPS2Keyboard() { Reset(); }
    void Reset();
    void KeyEvent(bool down, unsigned keycode);
    bool Pending() const { return !bytes.empty(); }
    uint8_t Front() const { return bytes.front(); }
    void Pop() { bytes.pop_front(); }

private:
    bool pressed[RETROK_LAST];
    std::deque<uint8_t> bytes;
};

#endif /* LIBRETRO_PS2_KEYBOARD_H */
