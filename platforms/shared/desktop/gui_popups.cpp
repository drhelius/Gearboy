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

#include <SDL3/SDL.h>

#define GUI_POPUPS_IMPORT
#include "gui_popups.h"
#include "gui.h"
#include "gui_actions.h"
#include "gui_colors.h"
#include "config.h"
#include "application.h"
#include "gamepad.h"
#include "emu.h"
#include "license.h"
#include "backers.h"
#include "ogl_renderer.h"
#include "keyboard.h"
#include "imgui.h"
#include "implot.h"
#include "../barcode_boy_codes.h"

static char build_info[4096] = "";
static int info_pos = 0;

static void add_build_info(const char* fmt, ...);
static void check_hotkey_duplicates_popup(config_Hotkey* current_hotkey);

void gui_popup_modal_keyboard()
{
    if (ImGui::BeginPopupModal("Keyboard Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        gui_dialog_in_use = true;
        ImGui::Text("Press any key to assign...\n\n");
        ImGui::Separator();

        SDL_Scancode scancode = keyboard_get_first_pressed_scancode();
        if (scancode != SDL_SCANCODE_UNKNOWN)
        {
            *gui_configured_key = scancode;
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void gui_popup_modal_gamepad(int pad)
{
    if (ImGui::BeginPopupModal("Gamepad Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        gui_dialog_in_use = true;
        SDL_Gamepad* controller = gamepad_controller[pad];

        if (IsValidPointer(controller))
            ImGui::Text("Press any button in your gamepad...\n\n");
        else
            ImGui::Text("No gamepad detected.\n\n");

        ImGui::Separator();

        if (IsValidPointer(controller))
        {
            for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
            {
                if (SDL_GetGamepadButton(controller, (SDL_GamepadButton)i))
                {
                    *gui_configured_button = i;
                    gui_dialog_in_use = false;
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }

            for (int a = SDL_GAMEPAD_AXIS_LEFTX; a < SDL_GAMEPAD_AXIS_COUNT; a++)
            {
                if (a != SDL_GAMEPAD_AXIS_LEFT_TRIGGER && a != SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
                    continue;

                Sint16 value = SDL_GetGamepadAxis(controller, (SDL_GamepadAxis)a);

                if (value > GAMEPAD_VBTN_AXIS_THRESHOLD)
                {
                    *gui_configured_button = GAMEPAD_VBTN_AXIS_BASE + a;
                    gui_dialog_in_use = false;
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void gui_popup_modal_hotkey()
{
    if (ImGui::BeginPopupModal("Hotkey Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        gui_dialog_in_use = true;
        ImGui::Text("Press any key combination...\n");
        ImGui::Text("Hold Ctrl, Shift, or Alt before pressing the key\n\n");
        ImGui::Separator();

        SDL_Keymod mods = (SDL_Keymod)(SDL_GetModState() & (SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT | SDL_KMOD_GUI));
        SDL_Scancode scancode = keyboard_get_first_pressed_scancode();
        if (scancode != SDL_SCANCODE_UNKNOWN)
        {
            gui_configured_hotkey->key = scancode;
            gui_configured_hotkey->mod = mods;
            config_update_hotkey_string(gui_configured_hotkey);
            check_hotkey_duplicates_popup(gui_configured_hotkey);
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static int barcode_character_filter(ImGuiInputTextCallbackData* data)
{
    return data->EventChar >= '0' && data->EventChar <= '9' ? 0 : 1;
}

void gui_popup_modal_barcode(void)
{
    if (ImGui::BeginPopupModal("Scan Barcode", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        gui_dialog_in_use = true;

        static char barcode[64] = "";
        static int selected_preset = -1;
        static const char* error_message = NULL;

        if (ImGui::IsWindowAppearing())
            error_message = NULL;

        float card_width = ImGui::CalcTextSize("Custom").x;
        float frame_padding = ImGui::GetStyle().FramePadding.x * 2.0f;

        for (int i = 0; kBarcodeBoyCodes[i].name; i++)
        {
            float width = ImGui::CalcTextSize(kBarcodeBoyCodes[i].name).x;
            card_width = MAX(card_width, width);
        }

        ImGui::TextUnformatted("Choose a card or enter a 13-digit barcode:");
        ImGui::SetNextItemWidth(card_width + frame_padding + ImGui::GetFrameHeight());

        if (ImGui::BeginCombo("##barcode_card", selected_preset < 0 ? "Custom" : kBarcodeBoyCodes[selected_preset].name))
        {
            if (ImGui::Selectable("Custom", selected_preset < 0))
                selected_preset = -1;

            for (int i = 0; kBarcodeBoyCodes[i].name; i++)
            {
                if (ImGui::Selectable(kBarcodeBoyCodes[i].name, selected_preset == i))
                {
                    selected_preset = i;
                    strcpy(barcode, kBarcodeBoyCodes[i].barcode);
                    error_message = NULL;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::NewLine();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Barcode:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::CalcTextSize("0000000000000").x + frame_padding);
        bool enter_pressed = ImGui::InputText("##barcode", barcode, sizeof(barcode),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter, barcode_character_filter);

        if (ImGui::IsItemEdited())
        {
            selected_preset = -1;
            error_message = NULL;
        }

        bool valid_barcode = strlen(barcode) == 13;

        if (barcode[0] && !valid_barcode)
            ImGui::TextDisabled("Enter exactly 13 digits (%d entered).", (int)strlen(barcode));

        if (error_message)
            ImGui::TextUnformatted(error_message);

        ImGui::NewLine();

        ImGui::BeginDisabled(!valid_barcode);
        bool scan_pressed = ImGui::Button("Scan", ImVec2(120, 0));
        ImGui::EndDisabled();

        if (valid_barcode && (scan_pressed || enter_pressed))
        {
            GB_BarcodeBoyResult result = emu_scan_barcode(barcode);

            if (result == GB_BarcodeBoyResult_Accepted)
            {
                char message[64];
                snprintf(message, sizeof(message), "Barcode queued: %s", barcode);
                gui_set_status_message(message, 3000);

                gui_dialog_in_use = false;
                ImGui::CloseCurrentPopup();
            }
            else if (result == GB_BarcodeBoyResult_Busy)
                error_message = "The reader is still sending the previous barcode.";
            else if (result == GB_BarcodeBoyResult_Unavailable)
                error_message = "Barcode Boy is not attached.";
            else
                error_message = "Enter exactly 13 decimal digits.";
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            gui_dialog_in_use = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void gui_popup_modal_about(void)
{
    if (ImGui::BeginPopupModal("About " GEARBOY_TITLE, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushFont(gui_default_font);
        ImGui::TextColored(cyan, "%s\n", GEARBOY_TITLE_ASCII);

        ImGui::TextColored(violet, "  By Ignacio Sánchez (DrHelius)");
        ImGui::Text(" "); ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://github.com/drhelius/" GEARBOY_TITLE);
        ImGui::Text(" "); ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://x.com/drhelius");
        ImGui::NewLine();

        ImGui::PopFont();

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Build Info"))
            {
                build_info[0] = '\0';
                info_pos = 0;

                add_build_info("Build: %s\n", GEARBOY_VERSION);
                #if defined(__DATE__) && defined(__TIME__)
                add_build_info("Built on: %s - %s\n", __DATE__, __TIME__);
                #endif
                #if defined(_M_ARM64)
                add_build_info("Windows ARM64 build\n");
                #endif
                #if defined(_M_X64)
                add_build_info("Windows 64 bit build\n");
                #endif
                #if defined(_M_IX86)
                add_build_info("Windows 32 bit build\n");
                #endif
                #if defined(__linux__) && defined(__x86_64__)
                add_build_info("Linux 64 bit build\n");
                #endif
                #if defined(__linux__) && defined(__i386__)
                add_build_info("Linux 32 bit build\n");
                #endif
                #if defined(__linux__) && defined(__arm__)
                add_build_info("Linux ARM build\n");
                #endif
                #if defined(__linux__) && defined(__aarch64__)
                add_build_info("Linux ARM64 build\n");
                #endif
                #if defined(__APPLE__) && defined(__arm64__ )
                add_build_info("macOS build (Apple Silicon)\n");
                #endif
                #if defined(__APPLE__) && defined(__x86_64__)
                add_build_info("macOS build (Intel)\n");
                #endif
                #if defined(__ANDROID__)
                add_build_info("Android build\n");
                #endif
                add_build_info("Config file: %s\n", config_emu_file_path);
                add_build_info("ImGui file: %s\n", config_imgui_file_path);
                add_build_info("Savestate version: %d\n", GB_SAVESTATE_VERSION);
                #if defined(_MSC_FULL_VER)
                add_build_info("Microsoft C++ %d\n", _MSC_FULL_VER);
                #endif
                #if defined(_MSVC_LANG)
                add_build_info("MSVC %d\n", _MSVC_LANG);
                #endif
                #if defined(__CLR_VER)
                add_build_info("CLR version: %d\n", __CLR_VER);
                #endif
                #if defined(__MINGW32__)
                add_build_info("MinGW 32 bit (%d.%d)\n", __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
                #endif
                #if defined(__MINGW64__)
                add_build_info("MinGW 64 bit (%d.%d)\n", __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR);
                #endif
                #if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
                add_build_info("GCC %d.%d.%d\n", (int)__GNUC__, (int)__GNUC_MINOR__, (int)__GNUC_PATCHLEVEL__);
                #endif
                #if defined(__clang_version__)
                add_build_info("Clang %s\n", __clang_version__);
                #endif
                add_build_info("SDL %d.%d.%d (build)\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
                add_build_info("SDL %d.%d.%d (link)\n", application_sdl_version_major, application_sdl_version_minor, application_sdl_version_patch);
                add_build_info("OpenGL %s\n", ogl_renderer_opengl_version);
                add_build_info("Dear ImGui %s (%d)\n", IMGUI_VERSION, IMGUI_VERSION_NUM);
                add_build_info("ImPlot %s (%d)\n", IMPLOT_VERSION, IMPLOT_VERSION_NUM);

                #if defined(DEBUG)
                add_build_info("define: DEBUG\n");
                #endif
                #if defined(NDEBUG)
                add_build_info("define: NDEBUG\n");
                #endif
                #if defined(DEBUG_GEARBOY)
                add_build_info("define: DEBUG_GEARBOY\n");
                #endif
                #if defined(GEARBOY_NO_OPTIMIZATIONS)
                add_build_info("define: GEARBOY_NO_OPTIMIZATIONS\n");
                #endif
                #if defined(GEARBOY_DISABLE_DISASSEMBLER)
                add_build_info("define: GEARBOY_DISABLE_DISASSEMBLER\n");
                #endif
                #if defined(__cplusplus)
                add_build_info("define: __cplusplus = %d\n", (int)__cplusplus);
                #endif
                #if defined(__STDC__)
                add_build_info("define: __STDC__ = %d\n", (int)__STDC__);
                #endif
                #if defined(__STDC_VERSION__)
                add_build_info("define: __STDC_VERSION__ = %d\n", (int)__STDC_VERSION__);
                #endif
                #if defined(IS_LITTLE_ENDIAN)
                add_build_info("define: IS_LITTLE_ENDIAN");
                #endif
                #if defined(IS_BIG_ENDIAN)
                add_build_info("define: IS_BIG_ENDIAN");
                #endif

                ImGui::InputTextMultiline("##build_info", build_info, sizeof(build_info), ImVec2(-1.0f, 100.0f), ImGuiInputTextFlags_ReadOnly);

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Special thanks to"))
            {
                ImGui::BeginChild("backers", ImVec2(0, 100), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                ImGui::Text("%s", BACKERS_STR);
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("LICENSE"))
            {
                ImGui::BeginChild("license", ImVec2(0, 100), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                ImGui::TextUnformatted(GPL_LICENSE_STR);
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::NewLine();
        ImGui::Separator();

        if (gamepad_added_mappings || gamepad_updated_mappings)
        {
            ImGui::Text("%d game controller mappings added from gamecontrollerdb.txt", gamepad_added_mappings);
            ImGui::Text("%d game controller mappings updated from gamecontrollerdb.txt", gamepad_updated_mappings);
        }
        else
            ImGui::Text("ERROR: Game controller database not found (gamecontrollerdb.txt)!!");

        ImGui::Separator();
        ImGui::NewLine();

        if (ImGui::Button("OK", ImVec2(120, 0))) 
        {
            ImGui::CloseCurrentPopup();
            gui_dialog_in_use = false;
        }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }
}

void gui_popup_modal_load_defaults(void)
{
    if (ImGui::BeginPopupModal("Load Default Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to load default settings?\n\n");
        ImGui::Text("The current game will reset.\n");
        ImGui::Text("Boot ROM configuration will be cleared and Link Cable will disconnect.\n");
        ImGui::Text("Active recordings will stop.\n\n");
        ImGui::Text("This action cannot be reverted.\n\n");
        ImGui::Separator();

        ImGui::BeginDisabled(gui_is_rom_loading() || emu_is_rom_loading());
        if (ImGui::Button("Yes", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            gui_dialog_in_use = false;
            gui_action_load_defaults();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            gui_dialog_in_use = false;
        }

        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void gui_show_info(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::Begin("ROM Info", &config_emulator.show_info, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    static char info[512] = "";
    emu_get_info(info, 512);

    ImGui::PushFont(gui_default_font);
    ImGui::SetCursorPosX(5.0f);
    ImGui::Text("%s", info);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_show_fps(void)
{
    ImGui::PushFont(gui_default_font);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,1.00f,0.0f,1.0f));
    ImGui::SetCursorPos(ImVec2(5.0f, config_debug.debug ? 25.0f : 5.0f));
    ImGui::Text("FPS:  %.2f\nTIME: %.2f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

static void add_build_info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(build_info + info_pos, sizeof(build_info) - info_pos, fmt, args);
    if (written > 0 && info_pos + written < (int)sizeof(build_info)) {
        info_pos += written;
    } else {
        info_pos = (int)sizeof(build_info) - 1;
        build_info[info_pos] = '\0';
    }
    va_end(args);
}

static void check_hotkey_duplicates_popup(config_Hotkey* current_hotkey)
{
    if (current_hotkey->key == SDL_SCANCODE_UNKNOWN)
        return;

    for (int i = 0; i < config_HotkeyIndex_COUNT; i++)
    {
        config_Hotkey* other = &config_hotkeys[i];

        if (other == current_hotkey)
            continue;

        if (other->key == current_hotkey->key &&
            other->mod == current_hotkey->mod)
        {
            other->key = SDL_SCANCODE_UNKNOWN;
            other->mod = SDL_KMOD_NONE;
            config_update_hotkey_string(other);
        }
    }
}
