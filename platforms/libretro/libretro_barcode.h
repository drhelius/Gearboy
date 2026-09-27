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

#ifndef LIBRETRO_BARCODE_H
#define LIBRETRO_BARCODE_H

#include "libretro.h"
#include "../../src/BarcodeBoy.h"

class GearboyCore;

void libretro_barcode_init(retro_environment_t environment_callback, retro_core_option_v2_definition* options);
void libretro_barcode_check_variables();
GB_BarcodeBoyMode libretro_barcode_get_mode();
bool libretro_barcode_load(GearboyCore* core);
void libretro_barcode_unload();
void libretro_barcode_update_input(u16 buttons);
size_t libretro_barcode_get_state_size();
bool libretro_barcode_save_state(void* data, size_t size);
bool libretro_barcode_load_state(const void* data, size_t size);

#endif /* LIBRETRO_BARCODE_H */
