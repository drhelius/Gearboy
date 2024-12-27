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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL.h>
#include "imgui.h"

static SDL_Keycode ImGuiKeyToSDLKeycode(ImGuiKey imgui_key);
static SDL_Keycode ImGuiKeyToSDLKeycode(ImGuiKey imgui_key)
{
    switch (imgui_key)
    {
        case ImGuiKey_Tab: return SDLK_TAB;
        case ImGuiKey_LeftArrow: return SDLK_LEFT;
        case ImGuiKey_RightArrow: return SDLK_RIGHT;
        case ImGuiKey_UpArrow: return SDLK_UP;
        case ImGuiKey_DownArrow: return SDLK_DOWN;
        case ImGuiKey_PageUp: return SDLK_PAGEUP;
        case ImGuiKey_PageDown: return SDLK_PAGEDOWN;
        case ImGuiKey_Home: return SDLK_HOME;
        case ImGuiKey_End: return SDLK_END;
        case ImGuiKey_Insert: return SDLK_INSERT;
        case ImGuiKey_Delete: return SDLK_DELETE;
        case ImGuiKey_Backspace: return SDLK_BACKSPACE;
        case ImGuiKey_Space: return SDLK_SPACE;
        case ImGuiKey_Enter: return SDLK_RETURN;
        case ImGuiKey_Escape: return SDLK_ESCAPE;
        case ImGuiKey_Apostrophe: return SDLK_QUOTE;
        case ImGuiKey_Comma: return SDLK_COMMA;
        case ImGuiKey_Minus: return SDLK_MINUS;
        case ImGuiKey_Period: return SDLK_PERIOD;
        case ImGuiKey_Slash: return SDLK_SLASH;
        case ImGuiKey_Semicolon: return SDLK_SEMICOLON;
        case ImGuiKey_Equal: return SDLK_EQUALS;
        case ImGuiKey_LeftBracket: return SDLK_LEFTBRACKET;
        case ImGuiKey_Backslash: return SDLK_BACKSLASH;
        case ImGuiKey_RightBracket: return SDLK_RIGHTBRACKET;
        case ImGuiKey_GraveAccent: return SDLK_BACKQUOTE;
        case ImGuiKey_CapsLock: return SDLK_CAPSLOCK;
        case ImGuiKey_ScrollLock: return SDLK_SCROLLLOCK;
        case ImGuiKey_NumLock: return SDLK_NUMLOCKCLEAR;
        case ImGuiKey_PrintScreen: return SDLK_PRINTSCREEN;
        case ImGuiKey_Pause: return SDLK_PAUSE;
        case ImGuiKey_Keypad0: return SDLK_KP_0;
        case ImGuiKey_Keypad1: return SDLK_KP_1;
        case ImGuiKey_Keypad2: return SDLK_KP_2;
        case ImGuiKey_Keypad3: return SDLK_KP_3;
        case ImGuiKey_Keypad4: return SDLK_KP_4;
        case ImGuiKey_Keypad5: return SDLK_KP_5;
        case ImGuiKey_Keypad6: return SDLK_KP_6;
        case ImGuiKey_Keypad7: return SDLK_KP_7;
        case ImGuiKey_Keypad8: return SDLK_KP_8;
        case ImGuiKey_Keypad9: return SDLK_KP_9;
        case ImGuiKey_KeypadDecimal: return SDLK_KP_PERIOD;
        case ImGuiKey_KeypadDivide: return SDLK_KP_DIVIDE;
        case ImGuiKey_KeypadMultiply: return SDLK_KP_MULTIPLY;
        case ImGuiKey_KeypadSubtract: return SDLK_KP_MINUS;
        case ImGuiKey_KeypadAdd: return SDLK_KP_PLUS;
        case ImGuiKey_KeypadEnter: return SDLK_KP_ENTER;
        case ImGuiKey_KeypadEqual: return SDLK_KP_EQUALS;
        case ImGuiKey_LeftCtrl: return SDLK_LCTRL;
        case ImGuiKey_LeftShift: return SDLK_LSHIFT;
        case ImGuiKey_LeftAlt: return SDLK_LALT;
        case ImGuiKey_LeftSuper: return SDLK_LGUI;
        case ImGuiKey_RightCtrl: return SDLK_RCTRL;
        case ImGuiKey_RightShift: return SDLK_RSHIFT;
        case ImGuiKey_RightAlt: return SDLK_RALT;
        case ImGuiKey_RightSuper: return SDLK_RGUI;
        case ImGuiKey_Menu: return SDLK_MENU;
        case ImGuiKey_0: return SDLK_0;
        case ImGuiKey_1: return SDLK_1;
        case ImGuiKey_2: return SDLK_2;
        case ImGuiKey_3: return SDLK_3;
        case ImGuiKey_4: return SDLK_4;
        case ImGuiKey_5: return SDLK_5;
        case ImGuiKey_6: return SDLK_6;
        case ImGuiKey_7: return SDLK_7;
        case ImGuiKey_8: return SDLK_8;
        case ImGuiKey_9: return SDLK_9;
        case ImGuiKey_A: return SDLK_a;
        case ImGuiKey_B: return SDLK_b;
        case ImGuiKey_C: return SDLK_c;
        case ImGuiKey_D: return SDLK_d;
        case ImGuiKey_E: return SDLK_e;
        case ImGuiKey_F: return SDLK_f;
        case ImGuiKey_G: return SDLK_g;
        case ImGuiKey_H: return SDLK_h;
        case ImGuiKey_I: return SDLK_i;
        case ImGuiKey_J: return SDLK_j;
        case ImGuiKey_K: return SDLK_k;
        case ImGuiKey_L: return SDLK_l;
        case ImGuiKey_M: return SDLK_m;
        case ImGuiKey_N: return SDLK_n;
        case ImGuiKey_O: return SDLK_o;
        case ImGuiKey_P: return SDLK_p;
        case ImGuiKey_Q: return SDLK_q;
        case ImGuiKey_R: return SDLK_r;
        case ImGuiKey_S: return SDLK_s;
        case ImGuiKey_T: return SDLK_t;
        case ImGuiKey_U: return SDLK_u;
        case ImGuiKey_V: return SDLK_v;
        case ImGuiKey_W: return SDLK_w;
        case ImGuiKey_X: return SDLK_x;
        case ImGuiKey_Y: return SDLK_y;
        case ImGuiKey_Z: return SDLK_z;
        case ImGuiKey_F1: return SDLK_F1;
        case ImGuiKey_F2: return SDLK_F2;
        case ImGuiKey_F3: return SDLK_F3;
        case ImGuiKey_F4: return SDLK_F4;
        case ImGuiKey_F5: return SDLK_F5;
        case ImGuiKey_F6: return SDLK_F6;
        case ImGuiKey_F7: return SDLK_F7;
        case ImGuiKey_F8: return SDLK_F8;
        case ImGuiKey_F9: return SDLK_F9;
        case ImGuiKey_F10: return SDLK_F10;
        case ImGuiKey_F11: return SDLK_F11;
        case ImGuiKey_F12: return SDLK_F12;
        case ImGuiKey_F13: return SDLK_F13;
        case ImGuiKey_F14: return SDLK_F14;
        case ImGuiKey_F15: return SDLK_F15;
        case ImGuiKey_F16: return SDLK_F16;
        case ImGuiKey_F17: return SDLK_F17;
        case ImGuiKey_F18: return SDLK_F18;
        case ImGuiKey_F19: return SDLK_F19;
        case ImGuiKey_F20: return SDLK_F20;
        case ImGuiKey_F21: return SDLK_F21;
        case ImGuiKey_F22: return SDLK_F22;
        case ImGuiKey_F23: return SDLK_F23;
        case ImGuiKey_F24: return SDLK_F24;
        case ImGuiKey_AppBack: return SDLK_AC_BACK;
        case ImGuiKey_AppForward: return SDLK_AC_FORWARD;
        default: return SDLK_UNKNOWN;
    }
}
#endif	/* KEYBOARD_H */