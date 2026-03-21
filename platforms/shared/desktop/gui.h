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

#ifndef GUI_H
#define GUI_H

#include <list>
#include <string>
#include <SDL3/SDL.h>
#include "gearboy.h"
#include "imgui.h"
#include "config.h"

#ifdef GUI_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum gui_ShortCutEvent
{
    gui_ShortcutOpenROM = 0,
    gui_ShortcutReloadROM,
    gui_ShortcutReset,
    gui_ShortcutPause,
    gui_ShortcutFFWD,
    gui_ShortcutSaveState,
    gui_ShortcutLoadState,
    gui_ShortcutScreenshot,
    gui_ShortcutDebugStepOver,
    gui_ShortcutDebugStepInto,
    gui_ShortcutDebugStepOut,
    gui_ShortcutDebugStepFrame,
    gui_ShortcutDebugBreak,
    gui_ShortcutDebugContinue,
    gui_ShortcutDebugRuntocursor,
    gui_ShortcutDebugGoBack,
    gui_ShortcutDebugBreakpoint,
    gui_ShortcutDebugCopy,
    gui_ShortcutDebugPaste,
    gui_ShortcutDebugSelectAll,
    gui_ShortcutShowMainMenu
};

struct gui_HotkeyMapping
{
    int shortcut;
    int config_index;
    bool allow_repeat;
};

#define GUI_HOTKEY_MAP_COUNT 18

const gui_HotkeyMapping gui_hotkey_map[GUI_HOTKEY_MAP_COUNT] = {
    {gui_ShortcutOpenROM, config_HotkeyIndex_OpenROM, false},
    {gui_ShortcutReloadROM, config_HotkeyIndex_ReloadROM, false},
    {gui_ShortcutReset, config_HotkeyIndex_Reset, false},
    {gui_ShortcutPause, config_HotkeyIndex_Pause, false},
    {gui_ShortcutFFWD, config_HotkeyIndex_FFWD, false},
    {gui_ShortcutSaveState, config_HotkeyIndex_SaveState, false},
    {gui_ShortcutLoadState, config_HotkeyIndex_LoadState, false},
    {gui_ShortcutScreenshot, config_HotkeyIndex_Screenshot, false},
    {gui_ShortcutShowMainMenu, config_HotkeyIndex_ShowMainMenu, false},
    {gui_ShortcutDebugStepInto, config_HotkeyIndex_DebugStepInto, true},
    {gui_ShortcutDebugStepOver, config_HotkeyIndex_DebugStepOver, true},
    {gui_ShortcutDebugStepOut, config_HotkeyIndex_DebugStepOut, true},
    {gui_ShortcutDebugStepFrame, config_HotkeyIndex_DebugStepFrame, true},
    {gui_ShortcutDebugContinue, config_HotkeyIndex_DebugContinue, true},
    {gui_ShortcutDebugBreak, config_HotkeyIndex_DebugBreak, true},
    {gui_ShortcutDebugRuntocursor, config_HotkeyIndex_DebugRunToCursor, false},
    {gui_ShortcutDebugBreakpoint, config_HotkeyIndex_DebugBreakpoint, false},
    {gui_ShortcutDebugGoBack, config_HotkeyIndex_DebugGoBack, false},
};

EXTERN bool gui_in_use;
EXTERN bool gui_main_window_hovered;
EXTERN bool gui_main_menu_hovered;
EXTERN ImFont* gui_default_font;
EXTERN ImFont* gui_default_fonts[4];
EXTERN ImFont* gui_roboto_font;
EXTERN ImFont* gui_material_icons_font;
EXTERN int gui_main_window_width;
EXTERN int gui_main_window_height;
EXTERN int gui_main_menu_height;
EXTERN SDL_Scancode* gui_configured_key;
EXTERN int* gui_configured_button;
EXTERN config_Hotkey* gui_configured_hotkey;
EXTERN bool gui_dialog_in_use;
EXTERN bool gui_shortcut_open_rom;
EXTERN std::list<std::string> gui_cheat_list;


EXTERN bool gui_init(void);
EXTERN void gui_destroy(void);
EXTERN void gui_render(void);
EXTERN void gui_shortcut(gui_ShortCutEvent event);
EXTERN void gui_load_rom(const char* path);
EXTERN void gui_set_status_message(const char* message, Uint64 milliseconds);
EXTERN void gui_set_error_message(const char* message);
Cartridge::CartridgeTypes gui_get_mbc(int index);

#undef GUI_IMPORT
#undef EXTERN
#endif /* GUI_H */