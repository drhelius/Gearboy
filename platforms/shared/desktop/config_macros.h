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

#ifndef CONFIG_MACROS_H
#define CONFIG_MACROS_H

#include "config.h"

#define CONFIG_BOOL(section, key, value, default_value) \
    process_bool(operation, section, key, &(value), default_value)
#define CONFIG_INT(section, key, value, default_value) \
    process_int(operation, section, key, &(value), default_value, false, 0, false, 0)
#define CONFIG_INT_MIN(section, key, value, default_value, minimum) \
    process_int(operation, section, key, &(value), default_value, true, minimum, false, 0)
#define CONFIG_INT_MAX(section, key, value, default_value, maximum) \
    process_int(operation, section, key, &(value), default_value, false, 0, true, maximum)
#define CONFIG_INT_RANGE(section, key, value, default_value, minimum, maximum) \
    process_int(operation, section, key, &(value), default_value, true, minimum, true, maximum)
#define CONFIG_FLOAT(section, key, value, default_value) \
    process_float(operation, section, key, &(value), default_value, false, 0.0f, false, 0.0f)
#define CONFIG_FLOAT_MIN(section, key, value, default_value, minimum) \
    process_float(operation, section, key, &(value), default_value, true, minimum, false, 0.0f)
#define CONFIG_FLOAT_MAX(section, key, value, default_value, maximum) \
    process_float(operation, section, key, &(value), default_value, false, 0.0f, true, maximum)
#define CONFIG_FLOAT_RANGE(section, key, value, default_value, minimum, maximum) \
    process_float(operation, section, key, &(value), default_value, true, minimum, true, maximum)
#define CONFIG_STRING(section, key, value, default_value) \
    process_string(operation, section, key, &(value), default_value, true)
#define CONFIG_STRING_NOT_EMPTY(section, key, value, default_value) \
    process_string(operation, section, key, &(value), default_value, false)
#define CONFIG_SCANCODE(section, key, value, default_value) \
    process_scancode(operation, section, key, &(value), default_value)
#define CONFIG_INT_ARRAY(section, key_format, values, count, default_value) \
    process_int_array(operation, section, key_format, values, count, default_value, false, 0, false, 0)
#define CONFIG_INT_ARRAY_MIN(section, key_format, values, count, default_value, minimum) \
    process_int_array(operation, section, key_format, values, count, default_value, true, minimum, false, 0)
#define CONFIG_INT_ARRAY_MAX(section, key_format, values, count, default_value, maximum) \
    process_int_array(operation, section, key_format, values, count, default_value, false, 0, true, maximum)
#define CONFIG_INT_ARRAY_RANGE(section, key_format, values, count, default_value, minimum, maximum) \
    process_int_array(operation, section, key_format, values, count, default_value, true, minimum, true, maximum)
#define CONFIG_FLOAT_ARRAY(section, key_format, values, count, default_value) \
    process_float_array(operation, section, key_format, values, count, default_value, false, 0.0f, false, 0.0f)
#define CONFIG_FLOAT_ARRAY_MIN(section, key_format, values, count, default_value, minimum) \
    process_float_array(operation, section, key_format, values, count, default_value, true, minimum, false, 0.0f)
#define CONFIG_FLOAT_ARRAY_MAX(section, key_format, values, count, default_value, maximum) \
    process_float_array(operation, section, key_format, values, count, default_value, false, 0.0f, true, maximum)
#define CONFIG_FLOAT_ARRAY_RANGE(section, key_format, values, count, default_value, minimum, maximum) \
    process_float_array(operation, section, key_format, values, count, default_value, true, minimum, true, maximum)
#define CONFIG_STRING_ARRAY(section, key_format, values, count, default_value) \
    process_string_array(operation, section, key_format, values, count, default_value, true)
#define CONFIG_STRING_ARRAY_NOT_EMPTY(section, key_format, values, count, default_value) \
    process_string_array(operation, section, key_format, values, count, default_value, false)
#define CONFIG_HOTKEY(key, value, default_key, default_mod) \
    process_hotkey(operation, key, &(value), default_key, default_mod)

#endif /* CONFIG_MACROS_H */
