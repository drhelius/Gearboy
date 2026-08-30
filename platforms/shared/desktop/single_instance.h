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

#ifndef SINGLE_INSTANCE_H
#define SINGLE_INSTANCE_H

#ifdef SINGLE_INSTANCE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void single_instance_init(const char* app_name);
EXTERN void single_instance_destroy(void);
EXTERN bool single_instance_try_lock(void);
EXTERN bool single_instance_is_primary(void);
EXTERN void single_instance_send_message(const char* rom_path, const char* symbol_path);
EXTERN void single_instance_poll(void);
EXTERN bool single_instance_get_pending_load(char* rom_path, int rom_path_size, char* symbol_path, int symbol_path_size);

#undef SINGLE_INSTANCE_IMPORT
#undef EXTERN
#endif /* SINGLE_INSTANCE_H */
