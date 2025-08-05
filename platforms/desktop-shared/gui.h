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
#define	GUI_H

#include "imgui/imgui.h"

#ifdef GUI_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum gui_ShortCutEvent
{
    gui_ShortcutOpenROM = 0,
    gui_ShortcutReset,
    gui_ShortcutPause,
    gui_ShortcutFFWD,
    gui_ShortcutSaveState,
    gui_ShortcutLoadState,
    gui_ShortcutScreenshot,
    gui_ShortcutDebugStep,
    gui_ShortcutDebugContinue,
    gui_ShortcutDebugNextFrame,
    gui_ShortcutDebugBreakpoint,
    gui_ShortcutDebugRuntocursor,
    gui_ShortcutDebugGoBack,
    gui_ShortcutDebugCopy,
    gui_ShortcutDebugPaste,
    gui_ShortcutShowMainMenu
};

EXTERN bool gui_in_use;
EXTERN bool gui_main_window_hovered;
EXTERN bool gui_main_menu_hovered;
EXTERN ImFont* gui_default_font;
EXTERN ImFont* gui_roboto_font;

EXTERN bool gui_init(void);
EXTERN void gui_destroy(void);
EXTERN void gui_render(void);
EXTERN void gui_shortcut(gui_ShortCutEvent event);
EXTERN void gui_load_rom(const char* path);
EXTERN void gui_set_status_message(const char* message, u32 milliseconds);

#undef GUI_IMPORT
#undef EXTERN
#endif	/* GUI_H */