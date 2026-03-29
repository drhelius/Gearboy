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
#include "emu.h"
#include "application.h"
#include "gamepad.h"

#define EVENTS_IMPORT
#include "events.h"

static bool input_updated = false;
static Uint16 input_last_state[1] = { };

static bool events_check_hotkey(const SDL_Event* event, const config_Hotkey& hotkey, bool allow_repeat);
static Uint16 input_build_state(int controller);
static void input_apply_state(int controller, Uint16 before, Uint16 now);

void events_shortcuts(const SDL_Event* event)
{
    if (event->type != SDL_EVENT_KEY_DOWN)
        return;

    // Check special case hotkeys first
    if (events_check_hotkey(event, config_hotkeys[config_HotkeyIndex_Quit], false))
    {
        application_trigger_quit();
        return;
    }

    if (events_check_hotkey(event, config_hotkeys[config_HotkeyIndex_Fullscreen], false))
    {
        config_emulator.fullscreen = !config_emulator.fullscreen;
        application_trigger_fullscreen(config_emulator.fullscreen);
        return;
    }

    if (events_check_hotkey(event, config_hotkeys[config_HotkeyIndex_CaptureMouse], false))
    {
        config_emulator.capture_mouse = !config_emulator.capture_mouse;
        return;
    }

    // Check slot selection hotkeys
    for (int i = 0; i < 5; i++)
    {
        if (events_check_hotkey(event, config_hotkeys[config_HotkeyIndex_SelectSlot1 + i], false))
        {
            config_emulator.save_slot = i;
            return;
        }
    }

    // Check all hotkeys mapped to gui shortcuts
    for (int i = 0; i < GUI_HOTKEY_MAP_COUNT; i++)
    {
        if (gui_hotkey_map[i].shortcut >= 0 && events_check_hotkey(event, config_hotkeys[gui_hotkey_map[i].config_index], gui_hotkey_map[i].allow_repeat))
        {
            gui_shortcut((gui_ShortCutEvent)gui_hotkey_map[i].shortcut);
            return;
        }
    }

    // Fixed hotkeys for debug copy/paste/select operations
    int key = event->key.scancode;
    SDL_Keymod mods = event->key.mod;

    if (event->key.repeat == 0 && key == SDL_SCANCODE_A && (mods & SDL_KMOD_CTRL))
    {
        gui_shortcut(gui_ShortcutDebugSelectAll);
        return;
    }

    if (event->key.repeat == 0 && key == SDL_SCANCODE_C && (mods & SDL_KMOD_CTRL))
    {
        gui_shortcut(gui_ShortcutDebugCopy);
        return;
    }

    if (event->key.repeat == 0 && key == SDL_SCANCODE_V && (mods & SDL_KMOD_CTRL))
    {
        gui_shortcut(gui_ShortcutDebugPaste);
        return;
    }

    // ESC to exit fullscreen
    if (event->key.repeat == 0 && key == SDL_SCANCODE_ESCAPE)
    {
        if (config_emulator.fullscreen && !config_emulator.always_show_menu)
        {
            config_emulator.fullscreen = false;
            application_trigger_fullscreen(false);
        }
    }
}

void events_handle_emu_event(const SDL_Event* event)
{
    if (gui_in_use)
        return;

    switch (event->type)
    {
        case SDL_EVENT_MOUSE_MOTION:
        {
            if ((config_emulator.tilt_source == 1) && (event->motion.xrel != 0.0f || event->motion.yrel != 0.0f))
            {
                int sen_x = config_emulator.mouse_sensitivity_x;
                int sen_y = config_emulator.mouse_sensitivity_y;
                if (sen_x < 1)
                    sen_x = 1;
                if (sen_y < 1)
                    sen_y = 1;

                float relx = -event->motion.xrel * ((float)sen_x / 200.0f);
                float rely = -event->motion.yrel * ((float)sen_y / 200.0f);

                if (config_emulator.mouse_invert_x)
                    relx = -relx;
                if (config_emulator.mouse_invert_y)
                    rely = -rely;

                emu_set_accelerometer(relx, rely);
            }

            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            break;
        }
    }
}

void events_emu(void)
{
    if (input_updated || gui_in_use)
        return;
    input_updated = true;

    SDL_PumpEvents();

    const int max_controller = 1;

    for (int controller = 0; controller < max_controller; controller++)
    {
        Uint16 now = input_build_state(controller);
        Uint16 before = input_last_state[controller];

        if (now != before)
            input_apply_state(controller, before, now);

        input_last_state[controller] = now;

        gamepad_check_shortcuts(controller);

        // Gamepad sensor tilt (Joy-Con, DualSense, etc.)
        if (config_emulator.tilt_source == 2)
        {
            SDL_Gamepad* sdl_controller = gamepad_controller[controller];
            if (IsValidPointer(sdl_controller) && SDL_GamepadHasSensor(sdl_controller, SDL_SENSOR_ACCEL))
            {
                if (!SDL_GamepadSensorEnabled(sdl_controller, SDL_SENSOR_ACCEL))
                    SDL_SetGamepadSensorEnabled(sdl_controller, SDL_SENSOR_ACCEL, true);

                float accel[3] = {};
                if (SDL_GetGamepadSensorData(sdl_controller, SDL_SENSOR_ACCEL, accel, 3))
                {
                    float gx = accel[0] / SDL_STANDARD_GRAVITY;
                    float gy = accel[1] / SDL_STANDARD_GRAVITY;
                    int sen_x = config_emulator.sensor_sensitivity_x;
                    int sen_y = config_emulator.sensor_sensitivity_y;
                    if (sen_x < 1)
                        sen_x = 1;
                    if (sen_y < 1)
                        sen_y = 1;
                    gx *= (float)sen_x / 5.0f;
                    gy *= (float)sen_y / 5.0f;
                    emu_set_accelerometer(gx, -gy);
                }
            }
        }
    }
}

void events_reset_input(void)
{
    input_updated = false;
}

bool events_input_updated(void)
{
    return input_updated;
}

static Uint16 input_build_state(int controller)
{
    if (controller < 0 || controller >= 1)
        return 0;

    SDL_Keymod mods = SDL_GetModState();
    if (mods & (SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT | SDL_KMOD_GUI))
        return 0;

    const bool* keyboard_state = SDL_GetKeyboardState(NULL);
    Uint16 ret = 0;

    if (keyboard_state[config_input.key_left])
        ret |= Left_Key;
    if (keyboard_state[config_input.key_right])
        ret |= Right_Key;
    if (keyboard_state[config_input.key_up])
        ret |= Up_Key;
    if (keyboard_state[config_input.key_down])
        ret |= Down_Key;
    if (keyboard_state[config_input.key_a])
        ret |= A_Key;
    if (keyboard_state[config_input.key_b])
        ret |= B_Key;
    if (keyboard_state[config_input.key_start])
        ret |= Start_Key;
    if (config_input.key_select != SDL_SCANCODE_UNKNOWN && keyboard_state[config_input.key_select])
        ret |= Select_Key;

    SDL_Gamepad* sdl_controller = gamepad_controller[controller];

    if (IsValidPointer(sdl_controller) && config_input.gamepad)
    {
        if (gamepad_get_button(sdl_controller, config_input.gamepad_a))
            ret |= A_Key;
        if (gamepad_get_button(sdl_controller, config_input.gamepad_b))
            ret |= B_Key;
        if (gamepad_get_button(sdl_controller, config_input.gamepad_start))
            ret |= Start_Key;
        if (gamepad_get_button(sdl_controller, config_input.gamepad_select))
            ret |= Select_Key;

        // Use D-Pad
        if (config_input.gamepad_directional == 0)
        {
            if (SDL_GetGamepadButton(sdl_controller, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
                ret |= Left_Key;
            if (SDL_GetGamepadButton(sdl_controller, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
                ret |= Right_Key;
            if (SDL_GetGamepadButton(sdl_controller, SDL_GAMEPAD_BUTTON_DPAD_UP))
                ret |= Up_Key;
            if (SDL_GetGamepadButton(sdl_controller, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
                ret |= Down_Key;
        }
        // Use analog sticks
        else
        {
            const Sint16 STICK_DEAD_ZONE = 8000;
            const Sint16 rawx = SDL_GetGamepadAxis(sdl_controller, (SDL_GamepadAxis)config_input.gamepad_x_axis);
            const Sint16 rawy = SDL_GetGamepadAxis(sdl_controller, (SDL_GamepadAxis)config_input.gamepad_y_axis);

            const Sint16 x = config_input.gamepad_invert_x_axis ? -rawx : rawx;
            const Sint16 y = config_input.gamepad_invert_y_axis ? -rawy : rawy;

            if (x < -STICK_DEAD_ZONE)
                ret |= Left_Key;
            else if (x > STICK_DEAD_ZONE)
                ret |= Right_Key;

            if (y < -STICK_DEAD_ZONE)
                ret |= Up_Key;
            else if (y > STICK_DEAD_ZONE)
                ret |= Down_Key;
        }
    }

    return ret;
}

static void input_apply_state(int controller, Uint16 before, Uint16 now)
{
    (void)controller;
    Uint16 pressed  = now & (Uint16)(~before);
    Uint16 released = before & (Uint16)(~now);

    if ((pressed | released) == 0)
        return;

    static const Uint16 keys[] = {
        Left_Key, Right_Key, Up_Key, Down_Key, A_Key, B_Key, Start_Key, Select_Key
    };

    for (unsigned i = 0; i < 8; i++)
    {
        Uint16 key = keys[i];
        if (pressed & key)  emu_key_pressed((Gameboy_Keys)key);
        if (released & key) emu_key_released((Gameboy_Keys)key);
    }
}

static bool events_check_hotkey(const SDL_Event* event, const config_Hotkey& hotkey, bool allow_repeat)
{
    if (event->type != SDL_EVENT_KEY_DOWN)
        return false;

    if (!allow_repeat && event->key.repeat != 0)
        return false;

    if (event->key.scancode != hotkey.key)
        return false;

    SDL_Keymod mods = event->key.mod;
    SDL_Keymod expected = hotkey.mod;

    SDL_Keymod mods_normalized = (SDL_Keymod)0;
    if (mods & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL)) mods_normalized = (SDL_Keymod)(mods_normalized | SDL_KMOD_CTRL);
    if (mods & (SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT)) mods_normalized = (SDL_Keymod)(mods_normalized | SDL_KMOD_SHIFT);
    if (mods & (SDL_KMOD_LALT | SDL_KMOD_RALT)) mods_normalized = (SDL_Keymod)(mods_normalized | SDL_KMOD_ALT);
    if (mods & (SDL_KMOD_LGUI | SDL_KMOD_RGUI)) mods_normalized = (SDL_Keymod)(mods_normalized | SDL_KMOD_GUI);

    SDL_Keymod expected_normalized = (SDL_Keymod)0;
    if (expected & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL | SDL_KMOD_CTRL)) expected_normalized = (SDL_Keymod)(expected_normalized | SDL_KMOD_CTRL);
    if (expected & (SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT | SDL_KMOD_SHIFT)) expected_normalized = (SDL_Keymod)(expected_normalized | SDL_KMOD_SHIFT);
    if (expected & (SDL_KMOD_LALT | SDL_KMOD_RALT | SDL_KMOD_ALT)) expected_normalized = (SDL_Keymod)(expected_normalized | SDL_KMOD_ALT);
    if (expected & (SDL_KMOD_LGUI | SDL_KMOD_RGUI | SDL_KMOD_GUI)) expected_normalized = (SDL_Keymod)(expected_normalized | SDL_KMOD_GUI);

    return mods_normalized == expected_normalized;
}
