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

#include <SDL3/SDL.h>
#include "imgui.h"

static bool keyboard_is_modifier_or_lang_scancode(SDL_Scancode sc)
{
    switch (sc)
    {
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_LGUI:
        case SDL_SCANCODE_RGUI:
        case SDL_SCANCODE_CAPSLOCK:
        case SDL_SCANCODE_NUMLOCKCLEAR:
        case SDL_SCANCODE_SCROLLLOCK:
        case SDL_SCANCODE_LANG1:
        case SDL_SCANCODE_LANG2:
        case SDL_SCANCODE_LANG3:
        case SDL_SCANCODE_LANG4:
        case SDL_SCANCODE_LANG5:
        case SDL_SCANCODE_LANG6:
        case SDL_SCANCODE_LANG7:
        case SDL_SCANCODE_LANG8:
        case SDL_SCANCODE_LANG9:
            return true;
        default:
            return false;
    }
}

static SDL_Scancode keyboard_get_first_pressed_scancode()
{
    int num_keys = 0;
    const bool* state = SDL_GetKeyboardState(&num_keys);
    for (int sc = 0; sc < num_keys; sc++)
    {
        if (state[sc] && !keyboard_is_modifier_or_lang_scancode((SDL_Scancode)sc))
            return (SDL_Scancode)sc;
    }
    return SDL_SCANCODE_UNKNOWN;
}

#endif /* KEYBOARD_H */