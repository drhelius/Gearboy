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

#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <SDL3/SDL.h>
#include "gearboy.h"

#ifdef GAMEPAD_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define GAMEPAD_VBTN_AXIS_BASE 1000
#define GAMEPAD_VBTN_AXIS_THRESHOLD 3000
#define GAMEPAD_VBTN_L2 (GAMEPAD_VBTN_AXIS_BASE + SDL_GAMEPAD_AXIS_LEFT_TRIGGER)
#define GAMEPAD_VBTN_R2 (GAMEPAD_VBTN_AXIS_BASE + SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)

struct Gamepad_Detected_Info
{
    SDL_JoystickID id;
    const char* name;
    char guid_str[64];
};

EXTERN SDL_Gamepad* gamepad_controller[GEARBOY_MAX_GAMEPADS];
EXTERN int gamepad_added_mappings;
EXTERN int gamepad_updated_mappings;

EXTERN bool gamepad_init(void);
EXTERN void gamepad_destroy(void);
EXTERN void gamepad_load_mappings(void);
EXTERN void gamepad_add(void);
EXTERN void gamepad_remove(SDL_JoystickID instance_id);
EXTERN void gamepad_assign(int slot, SDL_JoystickID instance_id);
EXTERN void gamepad_check_shortcuts(int controller);
EXTERN bool gamepad_get_button(SDL_Gamepad* controller, int mapping);
EXTERN int gamepad_get_detected(Gamepad_Detected_Info* out_list, int max_count);

#undef GAMEPAD_IMPORT
#undef EXTERN
#endif /* GAMEPAD_H */
