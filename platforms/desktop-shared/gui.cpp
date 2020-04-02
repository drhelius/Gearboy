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

#include "imgui/imgui.h"
#include "imgui/fonts/RobotoMedium.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "config.h"
#include "emu.h"
#include "renderer.h"
#include "application.h"

#define GUI_IMPORT
#include "gui.h"

static imgui_addons::ImGuiFileBrowser file_dialog;
static int main_menu_height;
static int main_window_width;
static int main_window_height;
static bool show_debug = false;
static bool dialog_in_use = false;
static SDL_Scancode* configured_key;
static SDL_GameControllerButton* configured_button;

static void main_menu(void);
static void main_window(void);
static void file_dialog_open_rom(void);
static void file_dialog_load_ram(void);
static void file_dialog_save_ram(void);
static void file_dialog_load_state(void);
static void file_dialog_save_state(void);
static void popup_modal_about(void);

void gui_init(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = config_imgui_file_path;
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(RobotoMedium_compressed_data_base85, 15.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
}

void gui_destroy(void)
{
    ImGui::DestroyContext();
}

void gui_render(void)
{
    ImGui::NewFrame();

    gui_in_use = dialog_in_use;

    main_menu();
    main_window();

    ImGui::Render();
}

static void main_menu(void)
{
    bool open_rom = false;
    bool open_ram = false;
    bool save_ram = false;
    bool open_state = false;
    bool save_state = false;
    bool open_about = false;
    
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game Boy"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Open ROM...", "Ctrl+O"))
            {
                open_rom = true;
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                emu_resume();
                emu_reset(config_emulator.force_dmg, config_emulator.save_in_rom_folder);

                if (config_emulator.start_paused)
                {
                    emu_pause();
                    
                    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
                        emu_frame_buffer[i] = 0;
                }
            }

            if (ImGui::MenuItem("Paused", "Ctrl+P", &config_emulator.paused))
            {
                if (emu_is_paused())
                    emu_resume();
                else
                    emu_pause();
            }

            if (ImGui::MenuItem("Fast Forward", "Ctrl+F", &config_emulator.ffwd))
            {
                config_audio.sync = !config_emulator.ffwd;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save RAM As...")) 
            {
                save_ram = true;
            }

            if (ImGui::MenuItem("Load RAM From..."))
            {
                open_ram = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save State As...")) 
            {
                save_state = true;
            }

            if (ImGui::MenuItem("Load State From..."))
            {
                open_state = true;
            }

            ImGui::Separator();
           
            if (ImGui::BeginMenu("Select State Slot"))
            {
                ImGui::Combo("", &config_emulator.save_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Save State", "Ctrl+S")) 
            {
                emu_save_state_slot(config_emulator.save_slot + 1);
            }

            if (ImGui::MenuItem("Load State", "Ctrl+L"))
            {
                emu_load_state_slot(config_emulator.save_slot + 1);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "Alt+F4"))
            {
                application_trigger_quit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator"))
        {
            gui_in_use = true;

            ImGui::MenuItem("Force DMG", "", &config_emulator.force_dmg);
            ImGui::MenuItem("Start Paused", "", &config_emulator.start_paused);
            ImGui::MenuItem("Save files in ROM folder", "", &config_emulator.save_in_rom_folder);
            
            if (ImGui::BeginMenu("Cheats"))
            {
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Video"))
        {
            gui_in_use = true;

            ImGui::MenuItem("Show FPS", "", &config_video.fps);
            ImGui::MenuItem("Bilinear Filtering", "", &config_video.bilinear);
            ImGui::MenuItem("Screen Ghosting", "", &config_video.mix_frames, false);
            ImGui::MenuItem("Dot Matrix", "", &config_video.matrix, false);
            
            ImGui::Separator();

            if (ImGui::BeginMenu("Palette"))
            {
                ImGui::Combo("", &config_emulator.save_slot, "Original\0Sharp\0Black & White\0Autumn\0Soft\0Slime\0Custom\0\0");
                ImGui::EndMenu();
            }

            ImGui::ColorEdit4("Color #1", (float*)&config_video.color[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
            ImGui::ColorEdit4("Color #2", (float*)&config_video.color[1], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
            ImGui::ColorEdit4("Color #3", (float*)&config_video.color[2], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
            ImGui::ColorEdit4("Color #4", (float*)&config_video.color[3], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Input"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("Keyboard Configuration"))
            {
                ImGui::Text("Left:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_left), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_left;
                    ImGui::OpenPopup("Keyboard Configuration");
                }
                                    
                ImGui::Text("Right:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_right), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_right;
                    ImGui::OpenPopup("Keyboard Configuration");
                }
                
                ImGui::Text("Up:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_up), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_up;
                    ImGui::OpenPopup("Keyboard Configuration");
                }

                ImGui::Text("Down:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_down), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_down;
                    ImGui::OpenPopup("Keyboard Configuration");
                }

                ImGui::Text("A:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_a), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_a;
                    ImGui::OpenPopup("Keyboard Configuration");
                }

                ImGui::Text("B:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_b), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_b;
                    ImGui::OpenPopup("Keyboard Configuration");
                }

                ImGui::Text("Start:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_start), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_start;
                    ImGui::OpenPopup("Keyboard Configuration");
                }
                
                ImGui::Text("Select:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GetScancodeName(config_input.key_select), ImVec2(70,0)))
                {
                    configured_key = &config_input.key_select;
                    ImGui::OpenPopup("Keyboard Configuration");
                }

                if (ImGui::BeginPopupModal("Keyboard Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Press any key...\n\n");
                    ImGui::Separator();

                    for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().KeysDown); i++)
                    {
                        if (ImGui::IsKeyPressed(i))
                        {
                            *configured_key = (SDL_Scancode)i;
                            ImGui::CloseCurrentPopup();
                            break;
                        }
                    } 

                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }                   
                
                ImGui::EndMenu();
            }

            ImGui::MenuItem("Enable Gamepad", "", &config_input.gamepad);
            
            if (ImGui::BeginMenu("Gamepad Configuration"))
            {
                /*
                ImGui::Text("Left:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_x_axis), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_x_axis;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("Right:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_x_axis), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_x_axis;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("Up:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_y_axis), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_y_axis;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("Down:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_y_axis), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_y_axis;
                    ImGui::OpenPopup("Gamepad Configuration");
                }
*/
                ImGui::Text("A:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_a), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_a;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("B:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_a), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_a;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("Start:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_start), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_start;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                ImGui::Text("Select:");
                ImGui::SameLine(70);
                if (ImGui::Button(SDL_GameControllerGetStringForButton(config_input.gamepad_select), ImVec2(70,0)))
                {
                    configured_button = &config_input.gamepad_select;
                    ImGui::OpenPopup("Gamepad Configuration");
                }

                if (ImGui::BeginPopupModal("Gamepad Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Press any button in your gamepad...\n\n");
                    ImGui::Separator();

                    for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().NavInputs); i++)
                    {
                        if (ImGui::GetIO().NavInputsDownDuration[i] == 0.0f)
                        {
                            *configured_key = (SDL_Scancode)i;
                            ImGui::CloseCurrentPopup();
                            break;
                        }
                    } 

                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }                   

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Enable", "", &config_audio.enable))
            {
                emu_audio_settings(config_audio.enable, 44100);
            }

            if (ImGui::MenuItem("Sync With Emulator", "", &config_audio.sync))
            {
                config_emulator.ffwd = false;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            gui_in_use = true;

            ImGui::MenuItem("Enabled", "", &show_debug, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("About"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("About " GEARBOY_TITLE " " GEARBOY_VERSION " ..."))
            {
               open_about = true;
            }
            ImGui::EndMenu();
        }

        main_menu_height = ImGui::GetWindowSize().y;

        ImGui::EndMainMenuBar();       
    }

    if (open_rom)
        ImGui::OpenPopup("Open ROM...");

    if (open_ram)
        ImGui::OpenPopup("Load RAM From...");

    if (save_ram)
        ImGui::OpenPopup("Save RAM As...");

    if (open_state)
        ImGui::OpenPopup("Load State From...");
    
    if (save_state)
        ImGui::OpenPopup("Save State As...");

    if (open_about)
    {
        dialog_in_use = true;
        ImGui::OpenPopup("About " GEARBOY_TITLE);
    }
    
    popup_modal_about();
    file_dialog_open_rom();
    file_dialog_load_ram();
    file_dialog_save_ram();
    file_dialog_load_state();
    file_dialog_save_state();
}

static void main_window(void)
{
    int w = ImGui::GetIO().DisplaySize.x;
    int h = ImGui::GetIO().DisplaySize.y - main_menu_height;

    int factor_w = w / GAMEBOY_WIDTH;
    int factor_h = h / GAMEBOY_HEIGHT;

    int factor = (factor_w < factor_h) ? factor_w : factor_h;

    main_window_width = GAMEBOY_WIDTH * factor;
    main_window_height = GAMEBOY_HEIGHT * factor;

    int window_x = (w - (GAMEBOY_WIDTH * factor)) / 2;
    int window_y = ((h - (GAMEBOY_HEIGHT * factor)) / 2) + main_menu_height;
    
    ImGui::SetNextWindowPos(ImVec2(window_x, window_y));
    ImGui::SetNextWindowSize(ImVec2(main_window_width, main_window_height));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(GEARBOY_TITLE, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);

    ImGui::Image((void*)(intptr_t)renderer_emu_texture, ImVec2(main_window_width,main_window_height));

    if (config_video.fps)
    {
        ImGui::SetCursorPos(ImVec2(5.0f, 5.0f));
        ImGui::Text("Frame Rate: %.2f FPS", ImGui::GetIO().Framerate);
        ImGui::SetCursorPosX(5.0f);
        ImGui::Text("Frame Time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);
    }

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

static void file_dialog_open_rom(void)
{
    if(file_dialog.showFileDialog("Open ROM...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), "*.*,.gb,.gbc,.cgb,.sgb,.dmg,.rom,.zip", &dialog_in_use))
    {
        emu_resume();
        emu_load_rom(file_dialog.selected_path.c_str(), config_emulator.force_dmg, config_emulator.save_in_rom_folder);

        if (config_emulator.start_paused)
        {
            emu_pause();
            
            for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
                emu_frame_buffer[i] = 0;
        }
    }
}

static void file_dialog_load_ram(void)
{
    if(file_dialog.showFileDialog("Load RAM From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".sav,*.*", &dialog_in_use))
    {
        emu_load_ram(file_dialog.selected_path.c_str(), config_emulator.force_dmg, config_emulator.save_in_rom_folder);
    }
}

static void file_dialog_save_ram(void)
{
    if(file_dialog.showFileDialog("Save RAM As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".sav", &dialog_in_use))
    {
        std::string state_path = file_dialog.selected_path;

        if (state_path.rfind(file_dialog.ext) != (state_path.size()-file_dialog.ext.size()))
        {
            state_path += file_dialog.ext;
        }

        emu_save_ram(state_path.c_str());
    }
}

static void file_dialog_load_state(void)
{
    if(file_dialog.showFileDialog("Load State From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".state,*.*", &dialog_in_use))
    {
        emu_load_state_file(file_dialog.selected_path.c_str());
    }
}

static void file_dialog_save_state(void)
{
    if(file_dialog.showFileDialog("Save State As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".state", &dialog_in_use))
    {
        std::string state_path = file_dialog.selected_path;

        if (state_path.rfind(file_dialog.ext) != (state_path.size()-file_dialog.ext.size()))
        {
            state_path += file_dialog.ext;
        }

        emu_save_state_file(state_path.c_str());
    }
}

static void popup_modal_about(void)
{
    if (ImGui::BeginPopupModal("About " GEARBOY_TITLE, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s %s", GEARBOY_TITLE, GEARBOY_VERSION);
        ImGui::Text("Build: %s", EMULATOR_BUILD);
        ImGui::Separator();
        ImGui::Text("By Ignacio Sánchez (twitter.com/drhelius)");
        ImGui::Text("%s is licensed under the GPL-3.0 License, see LICENSE for more information.", GEARBOY_TITLE);
        ImGui::Separator();        
        
        #ifdef _WIN32
        ImGui::Text("Windows 32 bit detected.");
        #endif
        #ifdef _WIN64
        ImGui::Text("Windows 32 bit detected.");
        #endif
        #ifdef __linux__
        ImGui::Text("Linux detected.");
        #endif
        #ifdef __APPLE__
        ImGui::Text("macOS detected.");
        #endif
        #ifdef _MSC_VER
        ImGui::Text("Built with Microsoft C++ %d.", _MSC_VER);
        #endif
        #ifdef __MINGW32__
        ImGui::Text("Built with MinGW 32 bit.");
        #endif
        #ifdef __MINGW64__
        ImGui::Text("Built with MinGW 64 bit.");
        #endif
        #if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
        ImGui::Text("Built with GCC %d.%d.%d", (int)__GNUC__, (int)__GNUC_MINOR__, (int)__GNUC_PATCHLEVEL__);
        #endif
        #ifdef __clang_version__
        ImGui::Text("Built with Clang %s.", __clang_version__);
        #endif
        #ifdef DEBUG
        ImGui::Text("define: DEBUG");
        #endif
        #ifdef DEBUG_GEARBOY
        ImGui::Text("define: DEBUG_GEARBOY");
        #endif
        ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) 
        {
            ImGui::CloseCurrentPopup();
            dialog_in_use = false;
        }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }
}
