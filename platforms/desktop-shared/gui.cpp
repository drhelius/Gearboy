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
#include "imgui/imgui_memory_editor.h"
#include "imgui/fonts/RobotoMedium.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "config.h"
#include "emu.h"
#include "../../src/gearboy.h"
#include "renderer.h"
#include "application.h"
#include "license.h"
#include "backers.h"
#include "gui_debug.h"

#define GUI_IMPORT
#include "gui.h"

static imgui_addons::ImGuiFileBrowser file_dialog;
static int main_menu_height;
static bool dialog_in_use = false;
static SDL_Scancode* configured_key;
static int* configured_button;
static ImVec4 custom_palette[4];
static std::list<std::string> cheat_list;
static bool shortcut_open_rom = false;
static ImFont* default_font[4];

static void main_menu(void);
static void main_window(void);
static void file_dialog_open_rom(void);
static void file_dialog_load_ram(void);
static void file_dialog_save_ram(void);
static void file_dialog_load_state(void);
static void file_dialog_save_state(void);
static void file_dialog_load_symbols(void);
static void keyboard_configuration_item(const char* text, SDL_Scancode* key);
static void gamepad_configuration_item(const char* text, int* button);
static void popup_modal_keyboard(void);
static void popup_modal_gamepad(void);
static void popup_modal_about(void);
static GB_Color color_float_to_int(ImVec4 color);
static ImVec4 color_int_to_float(GB_Color color);
static void update_palette(void);
static void push_recent_rom(std::string path);
static void menu_reset(void);
static void menu_pause(void);
static void menu_ffwd(void);
static void show_info(void);
static void show_fps(void);
static Cartridge::CartridgeTypes get_mbc(int index);

void gui_init(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = config_imgui_file_path;

    io.FontGlobalScale /= application_display_scale;

    gui_roboto_font = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 17.0f * application_display_scale, NULL, io.Fonts->GetGlyphRangesCyrillic());

    ImFontConfig font_cfg;

    for (int i = 0; i < 4; i++)
    {
        font_cfg.SizePixels = (13.0f + (i * 3)) * application_display_scale;
        default_font[i] = io.Fonts->AddFontDefault(&font_cfg);
    }

    gui_default_font = default_font[config_debug.font_size];

    update_palette();

    emu_audio_volume(config_audio.enable ? 1.0f: 0.0f);
    emu_color_correction(config_video.color_correction);
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

    if((!config_debug.debug && !emu_is_empty()) || (config_debug.debug && config_debug.show_gameboy))
        main_window();

    gui_debug_windows();

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
    case gui_ShortcutDebugStep:
        if (config_debug.debug)
            emu_debug_step();
        break;
    case gui_ShortcutDebugContinue:
        if (config_debug.debug)
            emu_debug_continue();
        break;
    case gui_ShortcutDebugNextFrame:
        if (config_debug.debug)
            emu_debug_next_frame();
        break;
    case gui_ShortcutDebugBreakpoint:
        if (config_debug.debug)
            gui_debug_toggle_breakpoint();
        break;
    case gui_ShortcutDebugRuntocursor:
        if (config_debug.debug)
            gui_debug_runtocursor();
        break;
    default:
        break;
    }
}

void gui_load_rom(const char* path)
{
    emu_resume();
    emu_load_rom(path, config_emulator.force_dmg, config_emulator.save_in_rom_folder, get_mbc(config_emulator.mbc), config_emulator.force_gba);
    cheat_list.clear();
    emu_clear_cheats();

    gui_debug_reset();

    std::string str(path);
    str = str.substr(0, str.find_last_of("."));
    str += ".sym";
    gui_debug_load_symbols_file(str.c_str());

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

static void main_menu(void)
{
    bool open_rom = false;
    bool open_ram = false;
    bool save_ram = false;
    bool open_state = false;
    bool save_state = false;
    bool open_about = false;
    bool open_symbols = false;

    for (int i = 0; i < 4; i++)
        custom_palette[i] = color_int_to_float(config_video.color[i]);
    
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(GEARBOY_TITLE))
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
                            gui_load_rom(config_emulator.recent_roms[i].c_str());
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

            ImGui::Separator();

            if (ImGui::MenuItem("Fast Forward", "Ctrl+F", &config_emulator.ffwd))
            {
                menu_ffwd();
            }

            if (ImGui::BeginMenu("Fast Forward Speed"))
            {
                ImGui::PushItemWidth(100.0f);
                ImGui::Combo("##fwd", &config_emulator.ffwd_speed, "X 1.5\0X 2\0X 2.5\0X 3\0Unlimited\0\0");
                ImGui::PopItemWidth();
                ImGui::EndMenu();
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
                ImGui::PushItemWidth(100.0f);
                ImGui::Combo("##slot", &config_emulator.save_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0\0");
                ImGui::PopItemWidth();
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

            if (ImGui::MenuItem("Quit", "ESC"))
            {
                application_trigger_quit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Force Game Boy (DMG)", "", &config_emulator.force_dmg))
            {
                if (config_emulator.force_dmg)
                    config_emulator.force_gba = false;
            }

            if (ImGui::MenuItem("Force Game Boy Advance", "", &config_emulator.force_gba))
            {
                if (config_emulator.force_gba)
                    config_emulator.force_dmg = false;
            }

            if (ImGui::BeginMenu("Memory Bank Controller"))
            {
                ImGui::PushItemWidth(140.0f);
                ImGui::Combo("##mbc", &config_emulator.mbc, "Auto\0ROM Only\0MBC 1\0MBC 2\0MBC 3\0MBC 5\0MBC 1 Multicart\0\0");
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem("Start Paused", "", &config_emulator.start_paused);
            
            ImGui::MenuItem("Save Files In ROM Folder", "", &config_emulator.save_in_rom_folder);

            ImGui::Separator();

            ImGui::MenuItem("Show ROM info", "", &config_emulator.show_info);

            ImGui::Separator();
            
            ImGui::SetNextWindowSizeConstraints({300.0f, 200.0f}, {300.0f, 500.0f});
            if (ImGui::BeginMenu("Cheats"))
            {
                ImGui::Text("Game Genie or GameShark codes\n(one code per line):");

                ImGui::Columns(2, "cheats", false);

                static char cheat_buffer[20*50] = "";
                ImGui::PushItemWidth(150);
                ImGui::InputTextMultiline("", cheat_buffer, IM_ARRAYSIZE(cheat_buffer));
                ImGui::PopItemWidth();

                ImGui::NextColumn();

                if (ImGui::Button("Add Cheat Codes"))
                {
                    std::string cheats = cheat_buffer;
                    std::istringstream ss(cheats);
                    std::string cheat;

                    while (getline(ss, cheat))
                    {
                        if ((cheat_list.size() < 50) && ((cheat.length() == 7) || (cheat.length() == 8) || (cheat.length() == 11)))
                        {
                            cheat_list.push_back(cheat);
                            emu_add_cheat(cheat.c_str());
                            cheat_buffer[0] = 0;
                        }
                    }
                }

                if (cheat_list.size() > 0)
                {
                    if (ImGui::Button("Clear All"))
                    {
                        cheat_list.clear();
                        emu_clear_cheats();
                    }
                }

                ImGui::Columns(1);

                std::list<std::string>::iterator it;

                for (it = cheat_list.begin(); it != cheat_list.end(); it++)
                {
                    if ((it->length() == 7) || (it->length() == 11))
                        ImGui::Text("Game Genie: %s", it->c_str());
                    else
                        ImGui::Text("GameShark: %s", it->c_str());
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
                ImGui::PushItemWidth(100.0f);
                ImGui::Combo("##scale", &config_video.scale, "Auto\0Zoom X1\0Zoom X2\0Zoom X3\0Zoom X4\0\0");
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Aspect Ratio"))
            {
                ImGui::PushItemWidth(130.0f);
                ImGui::Combo("##ratio", &config_video.ratio, "Game Boy\0Standard (4:3)\0Wide (16:9)\0Fit Window\0\0");
                ImGui::PopItemWidth();
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

            ImGui::Separator();

            ImGui::MenuItem("Bilinear Filtering", "", &config_video.bilinear);

            if (ImGui::MenuItem("Color Correction (GBC)", "", &config_video.color_correction))
            {
                emu_color_correction(config_video.color_correction);
            }

            if (ImGui::BeginMenu("Screen Ghosting"))
            {
                ImGui::MenuItem("Enable Screen Ghosting", "", &config_video.mix_frames);
                ImGui::SliderFloat("##screen_ghosting", &config_video.mix_frames_intensity, 0.0f, 1.0f, "Intensity = %.2f");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Dot Matrix"))
            {
                ImGui::MenuItem("Enable Dot Matrix", "", &config_video.matrix);
                ImGui::SliderFloat("##dot_matrix", &config_video.matrix_intensity, 0.0f, 1.0f, "Intensity = %.2f");
                ImGui::EndMenu();
            }
            
            ImGui::Separator();

            if (ImGui::BeginMenu("Palette"))
            {
                ImGui::PushItemWidth(130.0f);
                if (ImGui::Combo("##palette", &config_video.palette, "Original\0Sharp\0Black & White\0Autumn\0Soft\0Slime\0Custom\0\0"))
                {
                    update_palette();
                }
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            ImGui::Text("Custom Palette:");

            if (ImGui::ColorEdit3("Color #1", (float*)&custom_palette[0], ImGuiColorEditFlags_NoInputs))
            {
                update_palette();
            }
            if (ImGui::ColorEdit3("Color #2", (float*)&custom_palette[1], ImGuiColorEditFlags_NoInputs))
            {
                update_palette();
            }
            if (ImGui::ColorEdit3("Color #3", (float*)&custom_palette[2], ImGuiColorEditFlags_NoInputs))
            {
                update_palette();
            }
            if (ImGui::ColorEdit3("Color #4", (float*)&custom_palette[3], ImGuiColorEditFlags_NoInputs))
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

            if (ImGui::BeginMenu("Gamepad"))
            {
                ImGui::MenuItem("Enable Gamepad", "", &config_input.gamepad);

                if (ImGui::BeginMenu("Directional Controls"))
                {
                    ImGui::PushItemWidth(150.0f);
                    ImGui::Combo("##directional", &config_input.gamepad_directional, "D-pad\0Left Analog Stick\0\0");
                    ImGui::PopItemWidth();
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Button Configuration"))
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

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Enable Audio", "", &config_audio.enable))
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

        if (ImGui::BeginMenu("Debug"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Enable", "", &config_debug.debug))
            {
                if (config_debug.debug)
                    emu_debug_step();
                else
                    emu_debug_continue();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Step Over", "CTRL + F10", (void*)0, config_debug.debug))
            {
                emu_debug_step();
            }

            if (ImGui::MenuItem("Step Frame", "CTRL + F6", (void*)0, config_debug.debug))
            {
                emu_debug_next_frame();
            }

            if (ImGui::MenuItem("Continue", "CTRL + F5", (void*)0, config_debug.debug))
            {
                emu_debug_continue();
            }

            if (ImGui::MenuItem("Run To Cursor", "CTRL + F8", (void*)0, config_debug.debug))
            {
                gui_debug_runtocursor();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Toggle Breakpoint", "CTRL + F9", (void*)0, config_debug.debug))
            {
                gui_debug_toggle_breakpoint();
            }

            if (ImGui::MenuItem("Clear All Breakpoints", 0, (void*)0, config_debug.debug))
            {
                gui_debug_reset_breakpoints();
            }

            ImGui::MenuItem("Disable All Breakpoints", 0, &emu_debug_disable_breakpoints, config_debug.debug);

            ImGui::Separator();

            if (ImGui::BeginMenu("Font Size", config_debug.debug))
            {
                ImGui::PushItemWidth(110.0f);
                if (ImGui::Combo("##font", &config_debug.font_size, "Very Small\0Small\0Medium\0Large\0\0"))
                {
                    gui_default_font = default_font[config_debug.font_size];
                }
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem("Show Game Boy Screen", "", &config_debug.show_gameboy, config_debug.debug);

            ImGui::MenuItem("Show Disassembler", "", &config_debug.show_disassembler, config_debug.debug);

            ImGui::MenuItem("Show Processor Status", "", &config_debug.show_processor, config_debug.debug);

            ImGui::MenuItem("Show Memory Editor", "", &config_debug.show_memory, config_debug.debug);

            ImGui::MenuItem("Show IO Map", "", &config_debug.show_iomap, config_debug.debug);

            ImGui::MenuItem("Show VRAM Viewer", "", &config_debug.show_video, config_debug.debug);

            ImGui::MenuItem("Show Sound Registers", "", &config_debug.show_audio, config_debug.debug);

            ImGui::Separator();

            if (ImGui::MenuItem("Load Symbols...", "", (void*)0, config_debug.debug))
            {
                open_symbols = true;
            }

            if (ImGui::MenuItem("Clear Symbols", "", (void*)0, config_debug.debug))
            {
                gui_debug_reset_symbols();
            }

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

    if (open_symbols)
        ImGui::OpenPopup("Load Symbols File...");

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
    file_dialog_load_symbols();

    for (int i = 0; i < 4; i++)
        config_video.color[i] = color_float_to_int(custom_palette[i]);
}

static void main_window(void)
{
    int w = ImGui::GetIO().DisplaySize.x;
    int h = ImGui::GetIO().DisplaySize.y - main_menu_height;

    int selected_ratio = config_debug.debug ? 0 : config_video.ratio;
    float ratio = (float)GAMEBOY_WIDTH / (float)GAMEBOY_HEIGHT;

    switch (selected_ratio)
    {
        case 0:
            ratio = (float)GAMEBOY_WIDTH / (float)GAMEBOY_HEIGHT;
            break;
        case 1:
            ratio = 4.0f / 3.0f;
            break;
        case 2:
            ratio = 16.0f / 9.0f;
            break;
        case 3:
            ratio = (float)w / (float)h;
            break;
        default:
            ratio = (float)GAMEBOY_WIDTH / (float)GAMEBOY_HEIGHT;
    }

    int w_corrected = selected_ratio == 3 ? w : GAMEBOY_HEIGHT * ratio;
    int h_corrected = selected_ratio == 3 ? h : GAMEBOY_HEIGHT;

    int factor = 0;

    if (config_video.scale > 0)
    {
        factor = config_video.scale;
    }
    else if (config_debug.debug)
    {
        factor = 1;
    }
    else
    {
        int factor_w = w / w_corrected;
        int factor_h = h / h_corrected;
        factor = (factor_w < factor_h) ? factor_w : factor_h;
    }

    int main_window_width = w_corrected * factor;
    int main_window_height = h_corrected * factor;

    int window_x = (w - (w_corrected * factor)) / 2;
    int window_y = ((h - (h_corrected * factor)) / 2) + main_menu_height;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
    
    if (config_debug.debug)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::SetNextWindowPos(ImVec2(7, 32), ImGuiCond_FirstUseEver);

        ImGui::Begin(emu_is_cgb() ? "Game Boy Color###debug_output" : "Game Boy###debug_output", &config_debug.show_gameboy, flags);
    }
    else
    {
        ImGui::SetNextWindowSize(ImVec2(main_window_width, main_window_height));
        ImGui::SetNextWindowPos(ImVec2(window_x, window_y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

        ImGui::Begin(GEARBOY_TITLE, 0, flags);
    }

    ImGui::Image((void*)(intptr_t)renderer_emu_texture, ImVec2(main_window_width, main_window_height));

    if (config_video.fps)
        show_fps();

    if (config_emulator.show_info)
        show_info();

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    if (!config_debug.debug)
    {
        
        ImGui::PopStyleVar();
    }
}

static void file_dialog_open_rom(void)
{
    if(file_dialog.showFileDialog("Open ROM...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 400), "*.*,.gb,.gbc,.cgb,.sgb,.dmg,.rom,.zip", &dialog_in_use))
    {
        push_recent_rom(file_dialog.selected_path.c_str());
        gui_load_rom(file_dialog.selected_path.c_str());
    }
}

static void file_dialog_load_ram(void)
{
    if(file_dialog.showFileDialog("Load RAM From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".sav,*.*", &dialog_in_use))
    {
        emu_load_ram(file_dialog.selected_path.c_str(), config_emulator.force_dmg, config_emulator.save_in_rom_folder, get_mbc(config_emulator.mbc), config_emulator.force_gba);
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

static void file_dialog_load_symbols(void)
{
    if(file_dialog.showFileDialog("Load Symbols File...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 400), ".sym,*.*", &dialog_in_use))
    {
        gui_debug_reset_symbols();
        gui_debug_load_symbols_file(file_dialog.selected_path.c_str());
    }
}

static void keyboard_configuration_item(const char* text, SDL_Scancode* key)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(90);

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

    static const char* gamepad_names[16] = {"A", "B", "X" ,"Y", "BACK", "GUID", "START", "L3", "R3", "L1", "R1", "UP", "DOWN", "LEFT", "RIGHT", "15"};

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

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        {
            if (SDL_GameControllerGetButton(application_gamepad, (SDL_GameControllerButton)i))
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

        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
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

        ImGui::Separator();
        
        #ifdef _WIN64
        ImGui::Text("Windows 64 bit build");
        #elif defined(_WIN32)
        ImGui::Text("Windows 32 bit build");
        #endif
        #ifdef __linux__
        ImGui::Text("Linux build");
        #endif
        #ifdef __APPLE__
        ImGui::Text("macOS build");
        #endif
        #ifdef _MSC_VER
        ImGui::Text("Microsoft C++ %d.", _MSC_VER);
        #endif
        #ifdef __MINGW32__
        ImGui::Text("MinGW 32 bit (%d.%d)", __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
        #endif
        #ifdef __MINGW64__
        ImGui::Text("MinGW 64 bit (%d.%d)", __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR);
        #endif
        #if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
        ImGui::Text("GCC %d.%d.%d", (int)__GNUC__, (int)__GNUC_MINOR__, (int)__GNUC_PATCHLEVEL__);
        #endif
        #ifdef __clang_version__
        ImGui::Text("Clang %s", __clang_version__);
        #endif

        ImGui::Separator();

        #ifdef DEBUG
        ImGui::Text("define: DEBUG");
        #endif
        #ifdef DEBUG_GEARBOY
        ImGui::Text("define: DEBUG_GEARBOY");
        #endif
        #ifdef __cplusplus
        ImGui::Text("define: __cplusplus = %d", (int)__cplusplus);
        #endif
        #ifdef __STDC__
        ImGui::Text("define: __STDC__ = %d", (int)__STDC__);
        #endif
        #ifdef __STDC_VERSION__
        ImGui::Text("define: __STDC_VERSION__ = %d", (int)__STDC_VERSION__);
        #endif
        
        ImGui::Separator();

        ImGui::Text("SDL %d.%d.%d (build)", application_sdl_build_version.major, application_sdl_build_version.minor, application_sdl_build_version.patch);
        ImGui::Text("SDL %d.%d.%d (link) ", application_sdl_link_version.major, application_sdl_link_version.minor, application_sdl_link_version.patch);
        ImGui::Text("OpenGL %s", renderer_opengl_version);
        #ifndef __APPLE__
        ImGui::Text("GLEW %s", renderer_glew_version);
        #endif
        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

        ImGui::Separator();

        if (application_gamepad)
            ImGui::Text("Gamepad detected");
        else
            ImGui::Text("No gamepad detected");

        if (application_gamepad_mappings > 0)
            ImGui::Text("%d gamepad mappings loaded", application_gamepad_mappings);
        else
            ImGui::Text("Gamepad database not found");

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

static void push_recent_rom(std::string path)
{
    for (int i = (config_max_recent_roms - 1); i > 0; i--)
    {
        config_emulator.recent_roms[i] = config_emulator.recent_roms[i - 1];
    }

    config_emulator.recent_roms[0] = path;
}

static void menu_reset(void)
{
    emu_resume();
    emu_reset(config_emulator.force_dmg, config_emulator.save_in_rom_folder, get_mbc(config_emulator.mbc), config_emulator.force_gba);

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

static void show_info(void)
{
    if (config_video.fps)
        ImGui::SetCursorPosX(5.0f);
    else
        ImGui::SetCursorPos(ImVec2(5.0f, config_debug.debug ? 25.0f : 5.0f));

    static char info[512];

    emu_get_info(info);
    ImGui::Text("%s", info);
}

static void show_fps(void)
{
    ImGui::SetCursorPos(ImVec2(5.0f, config_debug.debug ? 25.0f : 5.0f ));
    ImGui::Text("Frame Rate: %.2f FPS\nFrame Time: %.2f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
}

static Cartridge::CartridgeTypes get_mbc(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeNotSupported;
        case 1:
            return Cartridge::CartridgeNoMBC;
        case 2:
            return Cartridge::CartridgeMBC1;
        case 3:
            return Cartridge::CartridgeMBC2;
        case 4:
            return Cartridge::CartridgeMBC3;
        case 5:
            return Cartridge::CartridgeMBC5;
        case 6:
            return Cartridge::CartridgeMBC1Multi;
        default:
            return Cartridge::CartridgeNotSupported;
    }
}
