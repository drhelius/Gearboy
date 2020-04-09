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
#include "../../src/gearboy.h"
#include "renderer.h"
#include "application.h"

#define GUI_IMPORT
#include "gui.h"

static imgui_addons::ImGuiFileBrowser file_dialog;
static int main_menu_height;
static int main_window_width;
static int main_window_height;
//static bool show_debug = false;
static bool dialog_in_use = false;
static SDL_Scancode* configured_key;
static int* configured_button;
static ImVec4 custom_palette[4];
static std::list<std::string> cheat_list;
static bool shortcut_open_rom = false;

static void main_menu(void);
static void main_window(void);
static void file_dialog_open_rom(void);
static void file_dialog_load_ram(void);
static void file_dialog_save_ram(void);
static void file_dialog_load_state(void);
static void file_dialog_save_state(void);
static void keyboard_configuration_item(const char* text, SDL_Scancode* key);
static void gamepad_configuration_item(const char* text, int* button);
static void popup_modal_keyboard(void);
static void popup_modal_gamepad(void);
static void popup_modal_about(void);
static GB_Color color_float_to_int(ImVec4 color);
static ImVec4 color_int_to_float(GB_Color color);
static void update_palette(void);
static void load_rom(const char* path);
static void push_recent_rom(std::string path);
static void menu_reset(void);
static void menu_pause(void);
static void menu_ffwd(void);

void gui_init(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = config_imgui_file_path;

    float font_scaling_factor = application_display_scale;
    float font_size = 17.0f;

    io.FontGlobalScale /= font_scaling_factor;

    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, font_size * font_scaling_factor, NULL, io.Fonts->GetGlyphRangesCyrillic());

    update_palette();
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

    if(!emu_is_empty())
        main_window();

    ImGui::Render();
}

void gui_shortcut(gui_ShortCutEvent event)
{
    switch (event)
    {  
    case gui_ShortcutOpenROM:
        shortcut_open_rom = true;
        break;
    case gui_ShortcutReset:
        menu_reset();
        break;
    case gui_ShortcutPause:
        menu_pause();
        break;
    case gui_ShortcutFFWD:
        config_emulator.ffwd = !config_emulator.ffwd;
        menu_ffwd();
        break;
    case gui_ShortcutSaveState:
        emu_save_state_slot(config_emulator.save_slot + 1);
        break;
    case gui_ShortcutLoadState:
        emu_load_state_slot(config_emulator.save_slot + 1);
        break;
    default:
        break;
    }
}

static void main_menu(void)
{
    bool open_rom = false;
    bool open_ram = false;
    bool save_ram = false;
    bool open_state = false;
    bool save_state = false;
    bool open_about = false;

    for (int i = 0; i < 4; i++)
        custom_palette[i] = color_int_to_float(config_video.color[i]);
    
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game Boy"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Open ROM...", "Ctrl+O"))
            {
                open_rom = true;
            }

            if (ImGui::BeginMenu("Open Recent"))
            {
                for (int i = 0; i < config_max_recent_roms; i++)
                {
                    if (config_emulator.recent_roms[i].length() > 0)
                    {
                        if (ImGui::MenuItem(config_emulator.recent_roms[i].c_str()))
                        {
                            load_rom(config_emulator.recent_roms[i].c_str());
                        }
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                menu_reset();
            }

            if (ImGui::MenuItem("Pause", "Ctrl+P", &config_emulator.paused))
            {
                menu_pause();
            }

            if (ImGui::MenuItem("Fast Forward", "Ctrl+F", &config_emulator.ffwd))
            {
                menu_ffwd();
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

            ImGui::MenuItem("Start Paused", "", &config_emulator.start_paused);

            ImGui::MenuItem("Force DMG Model", "", &config_emulator.force_dmg);
            
            ImGui::MenuItem("Save Files In ROM Folder", "", &config_emulator.save_in_rom_folder);

            ImGui::Separator();
            
            if (ImGui::BeginMenu("Cheats"))
            {
                ImGui::Text("Game Genie or GameShark codes:");

                static char cheat_buffer[12] = "";
                ImGui::PushItemWidth(150);
                ImGui::InputText("", cheat_buffer, 12);
                ImGui::PopItemWidth();
                ImGui::SameLine();

                if (ImGui::Button("Add Cheat Code"))
                {
                    std::string cheat = cheat_buffer;

                    if ((cheat_list.size() < 10) && ((cheat.length() == 7) || (cheat.length() == 8) || (cheat.length() == 11)))
                    {
                        cheat_list.push_back(cheat_buffer);
                        emu_add_cheat(cheat_buffer);
                        cheat_buffer[0] = 0;
                    }
                }

                std::list<std::string>::iterator it;

                for (it = cheat_list.begin(); it != cheat_list.end(); it++)
                {
                    if ((it->length() == 7) || (it->length() == 11))
                        ImGui::Text("Game Genie: %s", it->c_str());
                    else
                        ImGui::Text("GameShark: %s", it->c_str());
                }

                if (cheat_list.size() > 0)
                {
                    if (ImGui::Button("Clear All"))
                    {
                        cheat_list.clear();
                        emu_clear_cheats();
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Video"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("Scale"))
            {
                if (ImGui::Combo("", &config_video.scale, "Auto\0Zoom X1\0Zoom X2\0Zoom X3\0Zoom X4\0\0"))
                {
                    //update_palette();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Vertical Sync", "", &config_video.sync))
            {
                SDL_GL_SetSwapInterval(config_video.sync ? 1 : 0);

                if (config_video.sync)
                {
                    config_audio.sync = true;
                    emu_audio_reset();
                }
            }

            ImGui::MenuItem("Show FPS", "", &config_video.fps);
            ImGui::MenuItem("Bilinear Filtering", "", &config_video.bilinear);
            ImGui::MenuItem("Screen Ghosting", "", &config_video.mix_frames);
            //ImGui::MenuItem("Dot Matrix", "", &config_video.matrix, false);
            
            ImGui::Separator();

            if (ImGui::BeginMenu("Palette"))
            {
                if (ImGui::Combo("", &config_video.palette, "Original\0Sharp\0Black & White\0Autumn\0Soft\0Slime\0Custom\0\0"))
                {
                    update_palette();
                }
                ImGui::EndMenu();
            }

            ImGui::Text("Custom Palette:");

            if (ImGui::ColorEdit4("Color #1", (float*)&custom_palette[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
            {
                update_palette();
            }
            if (ImGui::ColorEdit4("Color #2", (float*)&custom_palette[1], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
            {
                update_palette();
            }
            if (ImGui::ColorEdit4("Color #3", (float*)&custom_palette[2], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
            {
                update_palette();
            }
            if (ImGui::ColorEdit4("Color #4", (float*)&custom_palette[3], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
            {
                update_palette();
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Input"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("Keyboard Configuration"))
            {
                keyboard_configuration_item("Left:", &config_input.key_left);
                keyboard_configuration_item("Right:", &config_input.key_right);
                keyboard_configuration_item("Up:", &config_input.key_up);
                keyboard_configuration_item("Down:", &config_input.key_down);
                keyboard_configuration_item("A:", &config_input.key_a);
                keyboard_configuration_item("B:", &config_input.key_b);
                keyboard_configuration_item("Start:", &config_input.key_start);
                keyboard_configuration_item("Select:", &config_input.key_select);

                popup_modal_keyboard();                 
                
                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem("Enable Gamepad", "", &config_input.gamepad);
            
            if (ImGui::BeginMenu("Gamepad Configuration"))
            {
                gamepad_configuration_item("A:", &config_input.gamepad_a);
                gamepad_configuration_item("B:", &config_input.gamepad_b);
                gamepad_configuration_item("START:", &config_input.gamepad_start);
                gamepad_configuration_item("SELECT:", &config_input.gamepad_select);

                popup_modal_gamepad();                 

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Enable", "", &config_audio.enable))
            {
                emu_audio_volume(config_audio.enable ? 1.0f: 0.0f);
            }

            if (ImGui::MenuItem("Sync With Emulator", "", &config_audio.sync))
            {
                config_emulator.ffwd = false;

                if (!config_audio.sync)
                {
                    config_video.sync = false;
                    SDL_GL_SetSwapInterval(0);
                }
            }

            ImGui::EndMenu();
        }

        // if (ImGui::BeginMenu("Debug"))
        // {
        //     gui_in_use = true;

        //     ImGui::MenuItem("Enabled", "", &show_debug, false);
        //     ImGui::EndMenu();
        // }

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

    if (open_rom || shortcut_open_rom)
    {
        shortcut_open_rom = false;
        ImGui::OpenPopup("Open ROM...");
    }

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

    for (int i = 0; i < 4; i++)
        config_video.color[i] = color_float_to_int(custom_palette[i]);
}

static void main_window(void)
{
    int w = ImGui::GetIO().DisplaySize.x;
    int h = ImGui::GetIO().DisplaySize.y - main_menu_height;

    int factor = 0;

    if (config_video.scale > 0)
    {
        factor = config_video.scale;
    }
    else
    {
        int factor_w = w / GAMEBOY_WIDTH;
        int factor_h = h / GAMEBOY_HEIGHT;
        factor = (factor_w < factor_h) ? factor_w : factor_h;
    }

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
    if(file_dialog.showFileDialog("Open ROM...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 400), "*.*,.gb,.gbc,.cgb,.sgb,.dmg,.rom,.zip", &dialog_in_use))
    {
        push_recent_rom(file_dialog.selected_path.c_str());
        load_rom(file_dialog.selected_path.c_str());
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

static void keyboard_configuration_item(const char* text, SDL_Scancode* key)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(70);

    char button_label[256];
    sprintf(button_label, "%s##%s", SDL_GetScancodeName(*key), text);

    if (ImGui::Button(button_label, ImVec2(70,0)))
    {
        configured_key = key;
        ImGui::OpenPopup("Keyboard Configuration");
    }
}

static void gamepad_configuration_item(const char* text, int* button)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(70);

    static const char* gamepad_names[16] = {"0", "A", "B" ,"3", "L", "R", "6", "7", "SELECT", "START", "10", "11", "12", "13", "14", "15"};

    char button_label[256];
    sprintf(button_label, "%s##%s", gamepad_names[*button], text);

    if (ImGui::Button(button_label, ImVec2(70,0)))
    {
        configured_button = button;
        ImGui::OpenPopup("Gamepad Configuration");
    }
}

static void popup_modal_keyboard(void)
{
    if (ImGui::BeginPopupModal("Keyboard Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Press any key...\n\n");
        ImGui::Separator();

        for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().KeysDown); i++)
        {
            if (ImGui::IsKeyPressed(i))
            {
                SDL_Scancode key = (SDL_Scancode)i;

                if ((key != SDL_SCANCODE_LCTRL) && (key != SDL_SCANCODE_RCTRL) && (key != SDL_SCANCODE_CAPSLOCK))
                {
                    *configured_key = key;
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void popup_modal_gamepad(void)
{
    if (ImGui::BeginPopupModal("Gamepad Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Press any button in your gamepad...\n\n");
        ImGui::Separator();

        for (int i = 0; i < 16; i++)
        {
            if (SDL_JoystickGetButton(application_gamepad, i))
            {
                *configured_button = i;
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
        
        #ifdef _WIN64
        ImGui::Text("Windows 64 bit detected.");
        #elif defined(_WIN32)
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

static GB_Color color_float_to_int(ImVec4 color)
{
    GB_Color ret;
    ret.red = floor(color.x >= 1.0 ? 255 : color.x * 256.0);
    ret.green = floor(color.y >= 1.0 ? 255 : color.y * 256.0);
    ret.blue = floor(color.z >= 1.0 ? 255 : color.z * 256.0);
    return ret;
}

static ImVec4 color_int_to_float(GB_Color color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 255.0f) * color.red;
    ret.y = (1.0f / 255.0f) * color.green;
    ret.z = (1.0f / 255.0f) * color.blue;
    return ret;
}

static void update_palette(void)
{
    if (config_video.palette == 6)
        emu_dmg_palette(config_video.color[0], config_video.color[1], config_video.color[2], config_video.color[3]);
    else
        emu_dmg_predefined_palette(config_video.palette);
}

static void load_rom(const char* path)
{
    emu_resume();
    emu_load_rom(path, config_emulator.force_dmg, config_emulator.save_in_rom_folder);
    cheat_list.clear();
    emu_clear_cheats();

    if (config_emulator.start_paused)
    {
        emu_pause();
        
        for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }
}

static void push_recent_rom(std::string path)
{
    for (int i = (config_max_recent_roms - 1); i >= 0; i--)
    {
        config_emulator.recent_roms[i] = config_emulator.recent_roms[i - 1];
    }

    config_emulator.recent_roms[0] = path;
}

static void menu_reset(void)
{
    emu_resume();
    emu_reset(config_emulator.force_dmg, config_emulator.save_in_rom_folder);

    if (config_emulator.start_paused)
    {
        emu_pause();
        
        for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }
}

static void menu_pause(void)
{
    if (emu_is_paused())
        emu_resume();
    else
        emu_pause();
}

static void menu_ffwd(void)
{
    config_audio.sync = !config_emulator.ffwd;

    if (config_emulator.ffwd)
        SDL_GL_SetSwapInterval(0);
    else
    {
        SDL_GL_SetSwapInterval(config_video.sync ? 1 : 0);
        emu_audio_reset();
    }
}
