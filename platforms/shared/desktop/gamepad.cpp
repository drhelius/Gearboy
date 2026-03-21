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

#include <SDL3/SDL.h>
#include "gearboy.h"
#include "config.h"
#include "gui.h"
#include "utils.h"

#define GAMEPAD_IMPORT
#include "gamepad.h"

static bool gamepad_shortcut_prev[GEARBOY_MAX_GAMEPADS][config_HotkeyIndex_COUNT] = { };

bool gamepad_init(void)
{
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
        InitPointer(gamepad_controller[i]);
    gamepad_load_mappings();
    gamepad_add();
    return true;
}

void gamepad_destroy(void)
{
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        if (IsValidPointer(gamepad_controller[i]))
        {
            SDL_CloseGamepad(gamepad_controller[i]);
        }
    }
}

void gamepad_load_mappings(void)
{
    std::string db_path;
    char exe_path[1024] = { };
    get_executable_path(exe_path, sizeof(exe_path));

    if (exe_path[0] != '\0')
    {
        db_path = std::string(exe_path) + "/gamecontrollerdb.txt";
    }
    else
    {
        db_path = "gamecontrollerdb.txt";
    }

    std::ifstream file;
    open_ifstream_utf8(file, db_path.c_str(), std::ios::in);
    if (!file.is_open())
    {
        open_ifstream_utf8(file, "gamecontrollerdb.txt", std::ios::in);
    }

    int added_mappings = 0;
    int updated_mappings = 0;
    int line_number = 0;
    char platform_field[64] = { };
    snprintf(platform_field, 64, "platform:%s", SDL_GetPlatform());

    if (file.is_open())
    {
        Debug("Loading gamecontrollerdb.txt file");
        std::string line;
        while (std::getline(file, line))
        {
            size_t comment = line.find_first_of('#');
            if (comment != std::string::npos)
                line = line.substr(0, comment);

            line = line.erase(0, line.find_first_not_of(" \t\r\n"));
            line = line.erase(line.find_last_not_of(" \t\r\n") + 1);

            while (line[0] == ' ')
                line = line.substr(1);

            if (line.empty())
                continue;

            size_t platform_pos = line.find("platform:Mac OS X");
            if (platform_pos != std::string::npos)
                line.replace(platform_pos, 17, "platform:macOS");

            if ((line.find("platform:") != std::string::npos) && (line.find(platform_field) == std::string::npos))
                continue;

            int result = SDL_AddGamepadMapping(line.c_str());

            if (result == 1)
                added_mappings++;
            else if (result == 0)
                updated_mappings++;
            else if (result == -1)
            {
                Error("Unable to load game controller mapping in line %d from gamecontrollerdb.txt", line_number);
                SDL_ERROR("SDL_AddGamepadMapping");
            }

            line_number++;
        }
        file.close();
    }
    else
    {
        Error("Game controller database not found (gamecontrollerdb.txt)!!");
        return;
    }

    Log("Added %d new game controller mappings from gamecontrollerdb.txt", added_mappings);
    Log("Updated %d game controller mappings from gamecontrollerdb.txt", updated_mappings);

    gamepad_added_mappings = added_mappings;
    gamepad_updated_mappings = updated_mappings;
}

void gamepad_add(void)
{
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        if (IsValidPointer(gamepad_controller[i]))
        {
            SDL_Joystick* js = SDL_GetGamepadJoystick(gamepad_controller[i]);

            if (!IsValidPointer(js) || !SDL_JoystickConnected(js))
            {
                SDL_CloseGamepad(gamepad_controller[i]);
                gamepad_controller[i] = NULL;
                Debug("Game controller %d closed when adding a new gamepad", i);
            }
        }
    }

    bool player_connected[GEARBOY_MAX_GAMEPADS] = { };

    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        player_connected[i] = IsValidPointer(gamepad_controller[i]);
    }

    bool all_slots_filled = true;
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        if (!player_connected[i])
        {
            all_slots_filled = false;
            break;
        }
    }
    if (all_slots_filled)
        return;

    int gamepad_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepad_count);

    for (int i = 0; i < gamepad_count; i++)
    {
        SDL_JoystickID joystick_id = gamepads[i];

        bool already_assigned = false;
        for (int p = 0; p < GEARBOY_MAX_GAMEPADS; p++)
        {
            if (player_connected[p])
            {
                SDL_JoystickID player_id = SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad_controller[p]));
                if (player_id == joystick_id)
                {
                    already_assigned = true;
                    break;
                }
            }
        }

        if (already_assigned)
            continue;

        SDL_Gamepad* controller = SDL_OpenGamepad(joystick_id);
        if (!IsValidPointer(controller))
        {
            Log("Warning: Unable to open game controller %d!\n", i);
            Log("SDL Error: SDL_OpenGamepad (%s:%d) - %s", __FILE__, __LINE__, SDL_GetError());
            SDL_ClearError();
            continue;
        }

        bool assigned = false;
        for (int p = 0; p < GEARBOY_MAX_GAMEPADS; p++)
        {
            if (!player_connected[p])
            {
                gamepad_controller[p] = controller;
                player_connected[p] = true;
                Debug("Game controller %d assigned to Player %d", i, p+1);
                assigned = true;
                break;
            }
        }

        if (!assigned)
        {
            SDL_CloseGamepad(controller);
            Debug("Game controller %d detected but all player slots are full", i);
        }

        all_slots_filled = true;
        for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
        {
            if (!player_connected[i])
            {
                all_slots_filled = false;
                break;
            }
        }
        if (all_slots_filled)
            break;
    }

    SDL_free(gamepads);
}

void gamepad_remove(SDL_JoystickID instance_id)
{
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        if (gamepad_controller[i] != NULL)
        {
            SDL_JoystickID current_id = SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad_controller[i]));
            if (current_id == instance_id)
            {
                SDL_CloseGamepad(gamepad_controller[i]);
                gamepad_controller[i] = NULL;
                Debug("Game controller %d disconnected from slot %d", instance_id, i);
                break;
            }
        }
    }
}

void gamepad_assign(int slot, SDL_JoystickID instance_id)
{
    if (slot < 0 || slot >= GEARBOY_MAX_GAMEPADS)
        return;

    if (instance_id == 0)
    {
        if (IsValidPointer(gamepad_controller[slot]))
        {
            SDL_CloseGamepad(gamepad_controller[slot]);
            gamepad_controller[slot] = NULL;
            Debug("Player %d controller set to None", slot + 1);
        }
        return;
    }

    if (IsValidPointer(gamepad_controller[slot]))
    {
        SDL_JoystickID current_id = SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad_controller[slot]));
        if (current_id == instance_id)
            return;
    }

    int other = -1;
    for (int i = 0; i < GEARBOY_MAX_GAMEPADS; i++)
    {
        if (i == slot)
            continue;

        if (IsValidPointer(gamepad_controller[i]))
        {
            SDL_JoystickID id = SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad_controller[i]));
            if (id == instance_id)
            {
                other = i;
                break;
            }
        }
    }

    if (other != -1)
    {
        if (IsValidPointer(gamepad_controller[slot]))
        {
            SDL_CloseGamepad(gamepad_controller[slot]);
            gamepad_controller[slot] = NULL;
        }

        gamepad_controller[slot] = gamepad_controller[other];
        gamepad_controller[other] = NULL;

        Debug("Moved controller from Player %d to Player %d", other + 1, slot + 1);
        return;
    }

    if (IsValidPointer(gamepad_controller[slot]))
    {
        SDL_CloseGamepad(gamepad_controller[slot]);
        gamepad_controller[slot] = NULL;
    }

    SDL_Gamepad* controller = SDL_OpenGamepad(instance_id);
    if (!IsValidPointer(controller))
    {
        Log("SDL_OpenGamepad failed for instance_id %d", instance_id);
        Log("SDL Error: SDL_OpenGamepad (%s:%d) - %s", __FILE__, __LINE__, SDL_GetError());
        SDL_ClearError();
        return;
    }

    gamepad_controller[slot] = controller;
    Debug("Game controller %d assigned to Player %d", instance_id, slot + 1);
}

int gamepad_get_detected(Gamepad_Detected_Info* out_list, int max_count)
{
    int gamepad_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepad_count);
    int result = 0;

    for (int i = 0; i < gamepad_count && result < max_count; i++)
    {
        Gamepad_Detected_Info info;
        info.id = gamepads[i];
        const char* name = SDL_GetGamepadNameForID(gamepads[i]);
        info.name = name ? name : "Unknown";
        SDL_GUID guid = SDL_GetJoystickGUIDForID(gamepads[i]);
        SDL_GUIDToString(guid, info.guid_str, sizeof(info.guid_str));
        out_list[result] = info;
        result++;
    }

    SDL_free(gamepads);
    return result;
}

void gamepad_check_shortcuts(int controller)
{
    SDL_Gamepad* sdl_controller = gamepad_controller[controller];
    if (!IsValidPointer(sdl_controller))
        return;

    for (int i = 0; i < config_HotkeyIndex_COUNT; i++)
    {
        int button_mapping = config_input_gamepad_shortcuts.gamepad_shortcuts[i];
        if (button_mapping == SDL_GAMEPAD_BUTTON_INVALID)
            continue;

        bool button_pressed = gamepad_get_button(sdl_controller, button_mapping);

        if (button_pressed && !gamepad_shortcut_prev[controller][i])
        {
            if (i >= config_HotkeyIndex_SelectSlot1 && i <= config_HotkeyIndex_SelectSlot5)
            {
                config_emulator.save_slot = i - config_HotkeyIndex_SelectSlot1;
            }
            else
            {
                for (int j = 0; j < GUI_HOTKEY_MAP_COUNT; j++)
                {
                    if (gui_hotkey_map[j].config_index == i)
                    {
                        gui_shortcut((gui_ShortCutEvent)gui_hotkey_map[j].shortcut);
                        break;
                    }
                }
            }
        }

        gamepad_shortcut_prev[controller][i] = button_pressed;
    }
}

bool gamepad_get_button(SDL_Gamepad* controller, int mapping)
{
    if (!IsValidPointer(controller))
        return false;

    if (mapping >= 0 && mapping < SDL_GAMEPAD_BUTTON_COUNT)
        return SDL_GetGamepadButton(controller, (SDL_GamepadButton)mapping) != 0;

    if (mapping >= GAMEPAD_VBTN_AXIS_BASE)
    {
        int axis = mapping - GAMEPAD_VBTN_AXIS_BASE;
        Sint16 value = SDL_GetGamepadAxis(controller, (SDL_GamepadAxis)axis);
        return value > GAMEPAD_VBTN_AXIS_THRESHOLD;
    }

    return false;
}
