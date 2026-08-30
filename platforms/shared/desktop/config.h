/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef CONFIG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#include "config_data.h"

enum config_Operation
{
    config_Operation_Defaults = 0,
    config_Operation_Read,
    config_Operation_Write
};

EXTERN const char* config_root_path;
EXTERN char config_emu_file_path[512];
EXTERN char config_imgui_file_path[512];

EXTERN void config_init(bool force_portable);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);
EXTERN void config_load_defaults(void);
EXTERN void config_push_recent_media(const std::string& path);
EXTERN void config_update_hotkey_string(config_Hotkey* hotkey);
EXTERN bool config_read_shader_parameter(const char* preset_file, const char* parameter_name, float* value);
EXTERN void config_write_shader_parameter(const char* preset_file, const char* parameter_name, float value);

#undef CONFIG_IMPORT
#undef EXTERN

#endif /* CONFIG_H */
