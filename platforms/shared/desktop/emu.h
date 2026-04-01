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

#ifndef EMU_H
#define EMU_H

#include "gearboy.h"

#ifdef EMU_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum Debug_Command
{
    Debug_Command_Continue,
    Debug_Command_Step,
    Debug_Command_StepFrame,
    Debug_Command_None
};

enum Directory_Location
{
    Directory_Location_Default = 0,
    Directory_Location_ROM = 1,
    Directory_Location_Custom = 2
};

EXTERN GB_Color* emu_frame_buffer;
EXTERN GB_Color* emu_debug_background_buffer;
EXTERN GB_Color* emu_debug_tile_buffers[2];
EXTERN GB_Color* emu_debug_oam_buffers[40];
EXTERN Debug_Command emu_debug_command;
EXTERN bool emu_debug_pc_changed;
EXTERN int emu_debug_step_frames_pending;
EXTERN int emu_debug_background_tile_address;
EXTERN int emu_debug_background_map_address;
EXTERN int emu_debug_tile_dmg_palette;
EXTERN int emu_debug_tile_color_palette;
EXTERN bool emu_debug_background_is_window;
EXTERN bool emu_audio_sync;
EXTERN bool emu_debug_disable_breakpoints;
EXTERN bool emu_debug_irq_breakpoints;

EXTERN bool emu_init(void);
EXTERN void emu_destroy(void);
EXTERN void emu_update(void);
EXTERN void emu_load_rom(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba);
EXTERN void emu_load_rom_async(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba);
EXTERN bool emu_is_rom_loading(void);
EXTERN bool emu_finish_rom_loading(void);
EXTERN void emu_key_pressed(Gameboy_Keys key);
EXTERN void emu_key_released(Gameboy_Keys key);
EXTERN void emu_pause(void);
EXTERN void emu_resume(void);
EXTERN bool emu_is_paused(void);
EXTERN bool emu_is_debug_idle(void);
EXTERN bool emu_is_empty(void);
EXTERN void emu_reset(bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba);
EXTERN void emu_audio_volume(float volume);
EXTERN void emu_audio_reset(void);
EXTERN bool emu_is_audio_enabled(void);
EXTERN bool emu_is_audio_open(void);
EXTERN void emu_dmg_palette(GB_Color& color1, GB_Color& color2, GB_Color& color3, GB_Color& color4);
EXTERN void emu_dmg_predefined_palette(int palette);
EXTERN bool emu_is_cgb(void);
EXTERN void emu_save_ram(const char* file_path);
EXTERN void emu_load_ram(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba);
EXTERN void emu_save_state_slot(int index);
EXTERN void emu_load_state_slot(int index);
EXTERN void emu_save_state_file(const char* file_path);
EXTERN void emu_load_state_file(const char* file_path);
EXTERN void update_savestates_data(void);
EXTERN GB_SaveState_Header emu_savestates[5];
EXTERN GB_SaveState_Screenshot emu_savestates_screenshots[5];
EXTERN void emu_add_cheat(const char* cheat);
EXTERN void emu_clear_cheats(void);
EXTERN void emu_get_info(char* info, int buffer_size);
EXTERN GearboyCore* emu_get_core(void);
EXTERN void emu_color_correction(bool correction);
EXTERN void emu_debug_step_over(void);
EXTERN void emu_debug_step_into(void);
EXTERN void emu_debug_step_out(void);
EXTERN void emu_debug_step_frame(void);
EXTERN void emu_debug_break(void);
EXTERN void emu_debug_continue(void);
EXTERN void emu_load_bootrom_dmg(const char* file_path);
EXTERN void emu_load_bootrom_gbc(const char* file_path);
EXTERN void emu_enable_bootrom_dmg(bool enable);
EXTERN void emu_enable_bootrom_gbc(bool enable);
EXTERN void emu_save_screenshot(const char* file_path);
EXTERN int emu_get_screenshot_png(unsigned char** out_buffer);
EXTERN int emu_get_sprite_png(int sprite_index, unsigned char** out_buffer);
EXTERN void emu_save_sprite(const char* file_path, int index);
EXTERN void emu_set_accelerometer(float x, float y, bool absolute);
EXTERN void emu_save_background(const char* file_path);
EXTERN void emu_save_tiles(const char* file_path);
EXTERN void emu_save_sgb_border(const char* file_path);
EXTERN void emu_save_sgb_tiles(const char* file_path, int palette);
EXTERN void emu_start_vgm_recording(const char* file_path);
EXTERN void emu_stop_vgm_recording(void);
EXTERN bool emu_is_vgm_recording(void);
EXTERN void emu_mcp_set_transport(int mode, int tcp_port);
EXTERN void emu_mcp_start(void);
EXTERN void emu_mcp_stop(void);
EXTERN bool emu_mcp_is_running(void);
EXTERN int emu_mcp_get_transport_mode(void);
EXTERN void emu_mcp_pump_commands(void);

#undef EMU_IMPORT
#undef EXTERN
#endif /* EMU_H */