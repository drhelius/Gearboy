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

#include "libretro_ps2_keyboard.h"
#include "../../src/GearboyCore.h"
#include "../shared/ps2_keyboard_scancodes.h"
#include <cstring>

static retro_environment_t environ_cb = NULL;
static GearboyCore* ps2_keyboard_core = NULL;
static GB_PS2KeyboardMode ps2_keyboard_mode = GB_PS2KeyboardMode_Auto;
static LibretroPS2Keyboard ps2_keyboard;

static void RETRO_CALLCONV ps2_keyboard_event(bool down, unsigned keycode,
    uint32_t character, uint16_t modifiers)
{
    (void)character;
    (void)modifiers;
    if (ps2_keyboard_core && ps2_keyboard_core->IsPS2KeyboardEnabled())
        ps2_keyboard.KeyEvent(down, keycode);
}

void libretro_ps2_keyboard_init(retro_environment_t environment_callback)
{
    environ_cb = environment_callback;
    struct retro_keyboard_callback ps2_keyboard_callback = { ps2_keyboard_event };
    environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &ps2_keyboard_callback);
}

void libretro_ps2_keyboard_check_variables()
{
    struct retro_variable variable = { "gearboy_ps2_keyboard", NULL };
    const char* mode_value = environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) ? variable.value : NULL;
    ps2_keyboard_mode = GB_PS2KeyboardMode_Auto;

    if (mode_value && strcmp(mode_value, "Disabled") == 0)
        ps2_keyboard_mode = GB_PS2KeyboardMode_Disabled;
    else if (mode_value && strcmp(mode_value, "Enabled") == 0)
        ps2_keyboard_mode = GB_PS2KeyboardMode_Enabled;
}

GB_PS2KeyboardMode libretro_ps2_keyboard_get_mode()
{
    return ps2_keyboard_mode;
}

bool libretro_ps2_keyboard_load(GearboyCore* core)
{
    ps2_keyboard_core = core;
    ps2_keyboard.Reset();
    return core && core->IsPS2KeyboardEnabled();
}

void libretro_ps2_keyboard_unload()
{
    ps2_keyboard_core = NULL;
    ps2_keyboard.Reset();
}

void libretro_ps2_keyboard_reset()
{
    ps2_keyboard.Reset();
}

void libretro_ps2_keyboard_update_input()
{
    if (ps2_keyboard_core && ps2_keyboard_core->IsPS2KeyboardEnabled())
    {
        while (ps2_keyboard.Pending() && ps2_keyboard_core->QueuePS2KeyboardScanCode(ps2_keyboard.Front()))
            ps2_keyboard.Pop();
    }
    else
        ps2_keyboard.Reset();
}

static uint8_t map_key(unsigned scancode)
{
    switch (scancode)
    {
        case RETROK_a: return PS2_SCANCODE_A;
        case RETROK_b: return PS2_SCANCODE_B;
        case RETROK_c: return PS2_SCANCODE_C;
        case RETROK_d: return PS2_SCANCODE_D;
        case RETROK_e: return PS2_SCANCODE_E;
        case RETROK_f: return PS2_SCANCODE_F;
        case RETROK_g: return PS2_SCANCODE_G;
        case RETROK_h: return PS2_SCANCODE_H;
        case RETROK_i: return PS2_SCANCODE_I;
        case RETROK_j: return PS2_SCANCODE_J;
        case RETROK_k: return PS2_SCANCODE_K;
        case RETROK_l: return PS2_SCANCODE_L;
        case RETROK_m: return PS2_SCANCODE_M;
        case RETROK_n: return PS2_SCANCODE_N;
        case RETROK_o: return PS2_SCANCODE_O;
        case RETROK_p: return PS2_SCANCODE_P;
        case RETROK_q: return PS2_SCANCODE_Q;
        case RETROK_r: return PS2_SCANCODE_R;
        case RETROK_s: return PS2_SCANCODE_S;
        case RETROK_t: return PS2_SCANCODE_T;
        case RETROK_u: return PS2_SCANCODE_U;
        case RETROK_v: return PS2_SCANCODE_V;
        case RETROK_w: return PS2_SCANCODE_W;
        case RETROK_x: return PS2_SCANCODE_X;
        case RETROK_y: return PS2_SCANCODE_Y;
        case RETROK_z: return PS2_SCANCODE_Z;
        case RETROK_1: return PS2_SCANCODE_1;
        case RETROK_2: return PS2_SCANCODE_2;
        case RETROK_3: return PS2_SCANCODE_3;
        case RETROK_4: return PS2_SCANCODE_4;
        case RETROK_5: return PS2_SCANCODE_5;
        case RETROK_6: return PS2_SCANCODE_6;
        case RETROK_7: return PS2_SCANCODE_7;
        case RETROK_8: return PS2_SCANCODE_8;
        case RETROK_9: return PS2_SCANCODE_9;
        case RETROK_0: return PS2_SCANCODE_0;
        case RETROK_ESCAPE: return PS2_SCANCODE_ESCAPE;
        case RETROK_BACKQUOTE: return PS2_SCANCODE_GRAVE;
        case RETROK_MINUS: return PS2_SCANCODE_MINUS;
        case RETROK_EQUALS: return PS2_SCANCODE_EQUALS;
        case RETROK_BACKSPACE: return PS2_SCANCODE_BACKSPACE;
        case RETROK_TAB: return PS2_SCANCODE_TAB;
        case RETROK_LEFTBRACKET: return PS2_SCANCODE_LEFTBRACKET;
        case RETROK_RIGHTBRACKET: return PS2_SCANCODE_RIGHTBRACKET;
        case RETROK_BACKSLASH: return PS2_SCANCODE_BACKSLASH;
        case RETROK_OEM_102: return PS2_SCANCODE_NONUSBACKSLASH;
        case RETROK_SEMICOLON: return PS2_SCANCODE_SEMICOLON;
        case RETROK_QUOTE: return PS2_SCANCODE_APOSTROPHE;
        case RETROK_RETURN: return PS2_SCANCODE_RETURN;
        case RETROK_LSHIFT: return PS2_SCANCODE_LSHIFT;
        case RETROK_RSHIFT: return PS2_SCANCODE_RSHIFT;
        case RETROK_LCTRL: return PS2_SCANCODE_LCTRL;
        case RETROK_LALT: return PS2_SCANCODE_LALT;
        case RETROK_SPACE: return PS2_SCANCODE_SPACE;
        case RETROK_COMMA: return PS2_SCANCODE_COMMA;
        case RETROK_PERIOD: return PS2_SCANCODE_PERIOD;
        case RETROK_SLASH: return PS2_SCANCODE_SLASH;
        case RETROK_F1: return PS2_SCANCODE_F1;
        case RETROK_F2: return PS2_SCANCODE_F2;
        case RETROK_F3: return PS2_SCANCODE_F3;
        case RETROK_F4: return PS2_SCANCODE_F4;
        case RETROK_F5: return PS2_SCANCODE_F5;
        case RETROK_F6: return PS2_SCANCODE_F6;
        case RETROK_F7: return PS2_SCANCODE_F7;
        case RETROK_F8: return PS2_SCANCODE_F8;
        case RETROK_F9: return PS2_SCANCODE_F9;
        case RETROK_F10: return PS2_SCANCODE_F10;
        case RETROK_F11: return PS2_SCANCODE_F11;
        case RETROK_F12: return PS2_SCANCODE_F12;
        case RETROK_CAPSLOCK: return PS2_SCANCODE_CAPSLOCK;
        case RETROK_SCROLLOCK: return PS2_SCANCODE_SCROLLLOCK;
        case RETROK_NUMLOCK: return PS2_SCANCODE_NUMLOCK;
        case RETROK_UP: return PS2_SCANCODE_UP;
        case RETROK_DOWN: return PS2_SCANCODE_DOWN;
        case RETROK_LEFT: return PS2_SCANCODE_LEFT;
        case RETROK_RIGHT: return PS2_SCANCODE_RIGHT;
        case RETROK_KP0: return PS2_SCANCODE_KP_0;
        case RETROK_KP1: return PS2_SCANCODE_KP_1;
        case RETROK_KP2: return PS2_SCANCODE_KP_2;
        case RETROK_KP3: return PS2_SCANCODE_KP_3;
        case RETROK_KP4: return PS2_SCANCODE_KP_4;
        case RETROK_KP5: return PS2_SCANCODE_KP_5;
        case RETROK_KP6: return PS2_SCANCODE_KP_6;
        case RETROK_KP7: return PS2_SCANCODE_KP_7;
        case RETROK_KP8: return PS2_SCANCODE_KP_8;
        case RETROK_KP9: return PS2_SCANCODE_KP_9;
        case RETROK_KP_PERIOD: return PS2_SCANCODE_KP_PERIOD;
        case RETROK_KP_DIVIDE: return PS2_SCANCODE_KP_DIVIDE;
        case RETROK_KP_MULTIPLY: return PS2_SCANCODE_KP_MULTIPLY;
        case RETROK_KP_MINUS: return PS2_SCANCODE_KP_MINUS;
        case RETROK_KP_PLUS: return PS2_SCANCODE_KP_PLUS;
        case RETROK_KP_ENTER: return PS2_SCANCODE_KP_ENTER;
        case RETROK_INSERT: return PS2_SCANCODE_INSERT;
        case RETROK_DELETE: return PS2_SCANCODE_DELETE;
        case RETROK_HOME: return PS2_SCANCODE_HOME;
        case RETROK_END: return PS2_SCANCODE_END;
        case RETROK_PAGEUP: return PS2_SCANCODE_PAGEUP;
        case RETROK_PAGEDOWN: return PS2_SCANCODE_PAGEDOWN;
        case RETROK_RCTRL: return PS2_SCANCODE_RCTRL;
        case RETROK_RALT: return PS2_SCANCODE_RALT;
        case RETROK_LMETA:
        case RETROK_LSUPER: return PS2_SCANCODE_LGUI;
        case RETROK_RMETA:
        case RETROK_RSUPER: return PS2_SCANCODE_RGUI;
        case RETROK_MENU: return PS2_SCANCODE_APPLICATION;
        default: return PS2_SCANCODE_UNKNOWN;
    }
}

static bool is_extended(unsigned scancode)
{
    switch (scancode)
    {
        case RETROK_UP:
        case RETROK_DOWN:
        case RETROK_LEFT:
        case RETROK_RIGHT:
        case RETROK_KP_DIVIDE:
        case RETROK_KP_ENTER:
        case RETROK_INSERT:
        case RETROK_DELETE:
        case RETROK_HOME:
        case RETROK_END:
        case RETROK_PAGEUP:
        case RETROK_PAGEDOWN:
        case RETROK_RCTRL:
        case RETROK_RALT:
        case RETROK_LMETA:
        case RETROK_LSUPER:
        case RETROK_RMETA:
        case RETROK_RSUPER:
        case RETROK_MENU:
            return true;
        default:
            return false;
    }
}

void LibretroPS2Keyboard::Reset()
{
    memset(pressed, 0, sizeof(pressed));
    bytes.clear();
}

void LibretroPS2Keyboard::KeyEvent(bool down, unsigned keycode)
{
    if (keycode == RETROK_UNKNOWN || keycode >= RETROK_LAST || pressed[keycode] == down)
        return;

    // Print Screen and Pause have special Set 2 sequences.
    if (keycode == RETROK_PRINT || keycode == RETROK_PAUSE)
    {
        pressed[keycode] = down;
        static const uint8_t print_down[] = { PS2_SCANCODE_EXTENDED, PS2_SCANCODE_LSHIFT, PS2_SCANCODE_EXTENDED, PS2_SCANCODE_PRINTSCREEN };
        static const uint8_t print_up[] = { PS2_SCANCODE_EXTENDED, PS2_SCANCODE_BREAK, PS2_SCANCODE_PRINTSCREEN, PS2_SCANCODE_EXTENDED, PS2_SCANCODE_BREAK, PS2_SCANCODE_LSHIFT };
        static const uint8_t pause[] = { PS2_SCANCODE_PAUSE_PREFIX, PS2_SCANCODE_LCTRL, PS2_SCANCODE_NUMLOCK, PS2_SCANCODE_PAUSE_PREFIX, PS2_SCANCODE_BREAK, PS2_SCANCODE_LCTRL, PS2_SCANCODE_BREAK, PS2_SCANCODE_NUMLOCK };
        if (keycode == RETROK_PRINT)
        {
            const uint8_t* sequence = down ? print_down : print_up;
            const unsigned size = down ? sizeof(print_down) : sizeof(print_up);
            bytes.insert(bytes.end(), sequence, sequence + size);
        }
        else if (down)
            bytes.insert(bytes.end(), pause, pause + sizeof(pause));
        return;
    }

    const uint8_t code = map_key(keycode);
    if (!code)
        return;

    pressed[keycode] = down;
    if (is_extended(keycode))
        bytes.push_back(PS2_SCANCODE_EXTENDED);
    if (!down)
        bytes.push_back(PS2_SCANCODE_BREAK);
    bytes.push_back(code);
}
