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

#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include "libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
 */

struct retro_core_option_v2_category option_cats_us[] = {
    {
        "system",
        "System",
        "Configure Game Boy model, memory bank controller and bootrom settings."
    },
    {
        "video",
        "Video",
        "Configure DMG palette and Game Boy Color correction settings."
    },
    {
        "input",
        "Input",
        "Configure controller behavior and directional input settings."
    },
    { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {

    /* System */

    {
        "gearboy_model",
        "Game Boy Model (restart)",
        NULL,
        "Select which hardware model is emulated. 'Auto' automatically selects the best model based on the loaded ROM header. 'Game Boy DMG' forces original Game Boy hardware. 'Game Boy Advance' forces Game Boy Advance mode, which enables minor hardware differences present when running Game Boy games on a GBA.",
        NULL,
        "system",
        {
            { "Auto",             NULL },
            { "Game Boy DMG",     NULL },
            { "Game Boy Advance", NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearboy_mapper",
        "Mapper (restart)",
        NULL,
        "Select which Memory Bank Controller (MBC or mapper) is emulated. 'Auto' automatically selects the best MBC based on the ROM header. Use manual selection only if a game has issues with automatic detection.",
        NULL,
        "system",
        {
            { "Auto",            NULL },
            { "ROM Only",        NULL },
            { "MBC 1",           NULL },
            { "MBC 2",           NULL },
            { "MBC 3",           NULL },
            { "MBC 5",           NULL },
            { "MBC 1 Multicart", NULL },
            { "HuC 1",           NULL },
            { "HuC 3",           NULL },
            { "MMM01",           NULL },
            { "Camera",          NULL },
            { "MBC 7",           NULL },
            { "TAMA5",           NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearboy_sgb",
        "Super Game Boy (restart)",
        NULL,
        "When enabled, compatible ROMs will run in Super Game Boy mode. When disabled, SGB-compatible ROMs will run as standard Game Boy (DMG) games.",
        NULL,
        "system",
        {
            { "Enabled",  NULL },
            { "Disabled", NULL },
            { NULL, NULL },
        },
        "Enabled"
    },
    {
        "gearboy_sgb_border",
        "Super Game Boy Border",
        NULL,
        "When enabled, the Super Game Boy border is displayed around the game screen (256x224). When disabled, only the game screen is shown (160x144).",
        NULL,
        "system",
        {
            { "Enabled",  NULL },
            { "Disabled", NULL },
            { NULL, NULL },
        },
        "Enabled"
    },
    {
        "gearboy_bootrom_dmg",
        "DMG Bootrom (restart)",
        NULL,
        "Enable or disable the original Game Boy (DMG) bootrom. When enabled, the bootrom will execute on startup just like on real hardware, showing the Nintendo logo scroll animation. The 'dmg_boot.bin' file must be present in RetroArch's system directory.",
        NULL,
        "system",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_bootrom_gbc",
        "GBC Bootrom (restart)",
        NULL,
        "Enable or disable the Game Boy Color bootrom. When enabled, the bootrom will execute on startup just like on real hardware, showing the Game Boy Color boot animation. The 'cgb_boot.bin' file must be present in RetroArch's system directory.",
        NULL,
        "system",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_tilt_source",
        "Tilt Source (MBC7)",
        NULL,
        "Select the input source for tilt control in MBC7 games (e.g. Kirby Tilt 'n' Tumble). 'Mouse' uses relative mouse movement. 'Sensor' uses the device's real accelerometer. 'Analog Stick' uses the left analog stick for tilt (D-Pad used for directions).",
        NULL,
        "input",
        {
            { "Mouse",        NULL },
            { "Sensor",       NULL },
            { "Analog Stick", NULL },
            { NULL, NULL },
        },
        "Mouse"
    },
    {
        "gearboy_sensor_sensitivity_x",
        "Sensor Sensitivity X (MBC7)",
        NULL,
        "Set the horizontal sensor sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_sensor_sensitivity_y",
        "Sensor Sensitivity Y (MBC7)",
        NULL,
        "Set the vertical sensor sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_sensor_invert_x",
        "Sensor Invert X (MBC7)",
        NULL,
        "Invert horizontal sensor axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_sensor_invert_y",
        "Sensor Invert Y (MBC7)",
        NULL,
        "Invert vertical sensor axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_mouse_sensitivity_x",
        "Mouse Sensitivity X (MBC7)",
        NULL,
        "Set the horizontal mouse sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_mouse_sensitivity_y",
        "Mouse Sensitivity Y (MBC7)",
        NULL,
        "Set the vertical mouse sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_mouse_invert_x",
        "Mouse Invert X (MBC7)",
        NULL,
        "Invert horizontal mouse axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_mouse_invert_y",
        "Mouse Invert Y (MBC7)",
        NULL,
        "Invert vertical mouse axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_analog_sensitivity_x",
        "Analog Sensitivity X (MBC7)",
        NULL,
        "Set the horizontal analog stick sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_analog_sensitivity_y",
        "Analog Sensitivity Y (MBC7)",
        NULL,
        "Set the vertical analog stick sensitivity for tilt control in MBC7 games.",
        NULL,
        "input",
        {
            { "1", NULL }, { "2", NULL }, { "3", NULL }, { "4", NULL }, { "5", NULL },
            { "6", NULL }, { "7", NULL }, { "8", NULL }, { "9", NULL }, { "10", NULL },
            { NULL, NULL },
        },
        "5"
    },
    {
        "gearboy_analog_invert_x",
        "Analog Invert X (MBC7)",
        NULL,
        "Invert horizontal analog stick axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearboy_analog_invert_y",
        "Analog Invert Y (MBC7)",
        NULL,
        "Invert vertical analog stick axis for tilt control.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },

    /* Video */

    {
        "gearboy_palette",
        "DMG Palette",
        NULL,
        "Select a color palette for original Game Boy (DMG) games. This setting has no effect on Game Boy Color games, which use their own built-in color palettes.",
        NULL,
        "video",
        {
            { "Original", NULL },
            { "Sharp",    NULL },
            { "B/W",      NULL },
            { "Autumn",   NULL },
            { "Soft",     NULL },
            { "Slime",    NULL },
            { NULL, NULL },
        },
        "Original"
    },
    {
        "gearboy_color_correction",
        "GBC Color Correction",
        NULL,
        "Apply color correction for Game Boy Color games to simulate the original GBC LCD screen output. When disabled, raw RGB colors are displayed which may appear overly saturated on modern displays.",
        NULL,
        "video",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },

    /* Input */

    {
        "gearboy_up_down_allowed",
        "Allow Up+Down / Left+Right",
        NULL,
        "Allow pressing, quickly alternating, or holding both left and right (or up and down) directions at the same time. This may cause movement based glitches in certain games. It's best to keep this option disabled.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },

    { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
    option_cats_us,
    option_defs_us
};

/*
 ********************************
 * Functions
 ********************************
 */

static void libretro_set_core_options(retro_environment_t environ_cb,
        bool *categories_supported)
{
    unsigned version = 0;

    if (!environ_cb || !categories_supported)
        return;

    *categories_supported = false;

    if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
        version = 0;

    if (version >= 2)
    {
        *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
                &options_us);
    }
    else
    {
        size_t i, j;
        size_t option_index  = 0;
        size_t num_options   = 0;
        struct retro_core_option_definition *option_v1_defs_us = NULL;
        struct retro_variable *variables   = NULL;
        char **values_buf                  = NULL;

        while (true)
        {
            if (option_defs_us[num_options].key)
                num_options++;
            else
                break;
        }

        if (version >= 1)
        {
            option_v1_defs_us = (struct retro_core_option_definition *)
                    calloc(num_options + 1, sizeof(struct retro_core_option_definition));

            for (i = 0; i < num_options; i++)
            {
                struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
                struct retro_core_option_value *option_values         = option_def_us->values;
                struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
                struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

                option_v1_def_us->key           = option_def_us->key;
                option_v1_def_us->desc          = option_def_us->desc;
                option_v1_def_us->info          = option_def_us->info;
                option_v1_def_us->default_value = option_def_us->default_value;

                while (option_values->value)
                {
                    option_v1_values->value = option_values->value;
                    option_v1_values->label = option_values->label;

                    option_values++;
                    option_v1_values++;
                }
            }

            environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
        }
        else
        {
            variables  = (struct retro_variable *)calloc(num_options + 1,
                    sizeof(struct retro_variable));
            values_buf = (char **)calloc(num_options, sizeof(char *));

            if (!variables || !values_buf)
                goto error;

            for (i = 0; i < num_options; i++)
            {
                const char *key                        = option_defs_us[i].key;
                const char *desc                       = option_defs_us[i].desc;
                const char *default_value              = option_defs_us[i].default_value;
                struct retro_core_option_value *values  = option_defs_us[i].values;
                size_t buf_len                         = 3;
                size_t default_index                   = 0;

                values_buf[i] = NULL;

                if (desc)
                {
                    size_t num_values = 0;

                    while (true)
                    {
                        if (values[num_values].value)
                        {
                            if (default_value)
                                if (strcmp(values[num_values].value, default_value) == 0)
                                    default_index = num_values;

                            buf_len += strlen(values[num_values].value);
                            num_values++;
                        }
                        else
                            break;
                    }

                    if (num_values > 0)
                    {
                        buf_len += num_values - 1;
                        buf_len += strlen(desc);

                        values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                        if (!values_buf[i])
                            goto error;

                        strcpy(values_buf[i], desc);
                        strcat(values_buf[i], "; ");

                        strcat(values_buf[i], values[default_index].value);

                        for (j = 0; j < num_values; j++)
                        {
                            if (j != default_index)
                            {
                                strcat(values_buf[i], "|");
                                strcat(values_buf[i], values[j].value);
                            }
                        }
                    }
                }

                variables[option_index].key   = key;
                variables[option_index].value = values_buf[i];
                option_index++;
            }

            environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
        }

error:
        if (option_v1_defs_us)
        {
            free(option_v1_defs_us);
            option_v1_defs_us = NULL;
        }

        if (values_buf)
        {
            for (i = 0; i < num_options; i++)
            {
                if (values_buf[i])
                {
                    free(values_buf[i]);
                    values_buf[i] = NULL;
                }
            }

            free(values_buf);
            values_buf = NULL;
        }

        if (variables)
        {
            free(variables);
            variables = NULL;
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
