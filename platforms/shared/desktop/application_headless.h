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

#ifndef APPLICATION_HEADLESS_H
#define APPLICATION_HEADLESS_H

int application_headless_init(const char* rom_file, const char* symbol_file, int mcp_mode, int mcp_tcp_port);
void application_headless_destroy(void);
void application_headless_mainloop(void);

#endif /* APPLICATION_HEADLESS_H */
