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

#include "libretro_barcode.h"
#include "../../src/GearboyCore.h"
#include "../shared/barcode_boy_codes.h"
#include <stdio.h>
#include <string.h>

static retro_environment_t environ_cb = NULL;
static GearboyCore* barcode_core = NULL;
static GB_BarcodeBoyMode barcode_mode = GB_BarcodeBoyMode_Auto;
static char barcode_digits[14] = "0000000000000";
static bool scan_button_pressed = true;
static const size_t barcode_state_header_size = 8;

static const char* get_variable(const char* key)
{
    struct retro_variable variable = { key, NULL };

    return environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) ? variable.value : NULL;
}

static bool update_option_display()
{
    bool enabled = barcode_core && barcode_core->IsBarcodeBoyEnabled();
    const char* preset = get_variable("gearboy_barcode");
    bool custom_barcode = !preset || strcmp(preset, "Custom") == 0;

    struct retro_core_option_display display = { "gearboy_barcode", enabled };
    environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &display);

    for (int i = 1; i <= 13; i++)
    {
        char option_key[40];
        snprintf(option_key, sizeof(option_key), "gearboy_barcode_digit_%d", i);

        display.key = option_key;
        display.visible = enabled && custom_barcode;
        environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &display);
    }

    return true;
}

void libretro_barcode_init(retro_environment_t environment_callback, retro_core_option_v2_definition* options)
{
    environ_cb = environment_callback;

    for (int i = 0; options[i].key; i++)
    {
        if (strcmp(options[i].key, "gearboy_barcode") != 0)
            continue;

        for (int j = 0; kBarcodeBoyCodes[j].name; j++)
        {
            options[i].values[j + 1].value = kBarcodeBoyCodes[j].name;
            options[i].values[j + 1].label = kBarcodeBoyCodes[j].name;
        }

        break;
    }

    struct retro_core_options_update_display_callback display_callback = { update_option_display };
    environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK, &display_callback);
}

void libretro_barcode_check_variables()
{
    const char* mode_value = get_variable("gearboy_barcode_boy");
    barcode_mode = GB_BarcodeBoyMode_Auto;

    if (mode_value && strcmp(mode_value, "Disabled") == 0)
        barcode_mode = GB_BarcodeBoyMode_Disabled;
    else if (mode_value && strcmp(mode_value, "Enabled") == 0)
        barcode_mode = GB_BarcodeBoyMode_Enabled;

    const char* preset = get_variable("gearboy_barcode");
    bool custom_barcode = true;

    for (int i = 0; preset && kBarcodeBoyCodes[i].name; i++)
    {
        if (strcmp(preset, kBarcodeBoyCodes[i].name) == 0)
        {
            memcpy(barcode_digits, kBarcodeBoyCodes[i].barcode, sizeof(barcode_digits));
            custom_barcode = false;

            break;
        }
    }

    if (custom_barcode)
    {
        for (int i = 0; i < 13; i++)
        {
            char option_key[40];
            snprintf(option_key, sizeof(option_key), "gearboy_barcode_digit_%d", i + 1);

            const char* digit = get_variable(option_key);
            barcode_digits[i] = digit && digit[0] >= '0' && digit[0] <= '9' && digit[1] == 0 ? digit[0] : '0';
        }

        barcode_digits[13] = 0;
    }

    update_option_display();
}

GB_BarcodeBoyMode libretro_barcode_get_mode()
{
    return barcode_mode;
}

bool libretro_barcode_load(GearboyCore* core)
{
    barcode_core = core;
    scan_button_pressed = true;
    update_option_display();

    return core && core->IsBarcodeBoyEnabled();
}

void libretro_barcode_unload()
{
    barcode_core = NULL;
    scan_button_pressed = true;
}

void libretro_barcode_update_input(u16 buttons)
{
    bool scan_pressed = (buttons & (1 << RETRO_DEVICE_ID_JOYPAD_R)) != 0;

    if (scan_pressed && !scan_button_pressed)
    {
        GB_BarcodeBoyResult result = barcode_core->ScanBarcode(barcode_digits);
        char message_text[96];

        if (result == GB_BarcodeBoyResult_Accepted)
            snprintf(message_text, sizeof(message_text), "Barcode queued: %s", barcode_digits);
        else if (result == GB_BarcodeBoyResult_Busy)
            snprintf(message_text, sizeof(message_text), "Barcode Boy is still sending the previous scan.");
        else
            snprintf(message_text, sizeof(message_text), "Barcode Boy is unavailable.");

        struct retro_message_ext message =
        {
            message_text, 2500, 1, RETRO_LOG_INFO, RETRO_MESSAGE_TARGET_ALL,
            RETRO_MESSAGE_TYPE_NOTIFICATION, -1
        };

        if (!environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &message))
        {
            struct retro_message legacy_message = { message_text, 150 };
            environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &legacy_message);
        }
    }

    scan_button_pressed = scan_pressed;
}

size_t libretro_barcode_get_state_size()
{
    size_t size = 0;

    if (!barcode_core->SaveState(NULL, size))
        return 0;

    return size + barcode_state_header_size;
}

bool libretro_barcode_save_state(void* data, size_t size)
{
    size_t required_size = libretro_barcode_get_state_size();

    if (!data || !required_size || size < required_size)
        return false;

    u8* state = (u8*)data;
    size_t core_size = required_size - barcode_state_header_size;

    if (!barcode_core->SaveState(state + barcode_state_header_size, core_size))
        return false;

    memcpy(state, "GBBC", 4);
    state[4] = scan_button_pressed ? 1 : 0;
    memset(state + 5, 0, 3);

    if (size > required_size)
        memset(state + required_size, 0, size - required_size);

    return true;
}

bool libretro_barcode_load_state(const void* data, size_t size)
{
    if (!data || size < barcode_state_header_size)
        return false;

    const u8* state = (const u8*)data;

    if (memcmp(state, "GBBC", 4) != 0)
    {
        if (!barcode_core->LoadState(state, size))
            return false;

        scan_button_pressed = true;

        return true;
    }

    if (state[4] > 1 || state[5] || state[6] || state[7])
        return false;

    const u8* core_state = state + barcode_state_header_size;
    size_t core_size = size - barcode_state_header_size;

    while (core_size >= sizeof(GB_SaveState_Header_Libretro))
    {
        GB_SaveState_Header_Libretro header;
        memcpy(&header, core_state + core_size - sizeof(header), sizeof(header));

        if (header.magic == GB_SAVESTATE_MAGIC &&
            header.version >= GB_SAVESTATE_MIN_VERSION && header.version <= GB_SAVESTATE_VERSION)
            break;

        if (core_state[core_size - 1] != 0)
            return false;

        core_size--;
    }

    if (core_size < sizeof(GB_SaveState_Header_Libretro) || !barcode_core->LoadState(core_state, core_size))
        return false;

    scan_button_pressed = state[4] != 0;

    return true;
}
