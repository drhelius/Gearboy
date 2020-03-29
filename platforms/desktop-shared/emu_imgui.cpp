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

#include <SDL.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <SDL_opengl.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"
#include "FileBrowser/ImGuiFileBrowser.h"

#include "emu_sdl.h"
#include "Emulator.h"
#include "config.h"

#define EMU_IMGUI_IMPORT
#include "emu_imgui.h"

static void gui_main_menu(void);
static void gui_main_window(void);
static void gui_about_window(void);
static void emu_run(void);
static void emu_update_begin(void);
static void emu_update_end(void);
static void emu_frame_throttle(float min);

static imgui_addons::ImGuiFileBrowser file_dialog;

static Emulator* emu;
static u16* emu_frame_buffer;
static GLuint emu_texture;
static Uint64 emu_start_frame_time;

static int gui_main_menu_height;
static int gui_main_window_width;
static int gui_main_window_height;
static bool gui_show_about_window = false;
static bool gui_show_debug = false;

void emu_imgui_init(void)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(emu_sdl_window, emu_sdl_gl_context);
    ImGui_ImplOpenGL2_Init();

    emu_frame_buffer = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];

    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        emu_frame_buffer[i] = 0;

    emu = new Emulator();
    emu->Init();

    config_read();

    ImGui::GetIO().IniFilename = config_imgui_file_path;

#ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        Log("GLEW Error: %s\n", glewGetErrorString(err));
    }
    Log("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    //glGenFramebuffers(1, &emu_fbo);
    glGenTextures(1, &emu_texture);  
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, emu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);
}

void emu_imgui_destroy(void)
{
    config_write();

    SafeDelete(emu);
    SafeDeleteArray(emu_frame_buffer);

    glDeleteTextures(1, &emu_texture);
    
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();
}

void emu_imgui_update(void)
{
    emu_update_begin();

    emu_run();

    gui_main_menu();
    gui_main_window();
    if (gui_show_about_window)
        gui_about_window();

    //bool show_demo_window = true;
    //ImGui::ShowDemoWindow(&show_demo_window);

    emu_update_end();

    if (emu->IsEmpty() || emu->IsPaused() || !emu->IsAudioEnabled() || config_emulator_options.ffwd)
    {
        emu_frame_throttle(config_emulator_options.ffwd ? 8.0f : 16.666f);
    }
}

void emu_imgui_event(const SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

static void emu_run(void)
{
    config_emulator_options.paused = emu->IsPaused();

    emu->RunToVBlank(emu_frame_buffer, config_audio_options.sync);

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, emu_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);

    if (config_video_options.bilinear)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void emu_update_begin(void)
{
    emu_start_frame_time = SDL_GetPerformanceCounter();

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(emu_sdl_window);

    ImGui::NewFrame();
}

static void emu_update_end(void)
{
    ImGui::Render();
    
    ImVec4 clear_color = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void emu_frame_throttle(float min)
{
    Uint64 end_time = SDL_GetPerformanceCounter();

    float elapsedMS = (float)((end_time - emu_start_frame_time) * 1000) / SDL_GetPerformanceFrequency();

    if (elapsedMS < min)
	    SDL_Delay((Uint32)(min - elapsedMS));
}

static void gui_main_menu(void)
{
    bool open_rom = false;
    bool open_state = false;
    bool save_state = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game Boy"))
        {
            if (ImGui::MenuItem("Open ROM...", "Ctrl+O"))
            {
                open_rom = true;
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                emu->Resume();
                emu->Reset(config_emulator_options.force_dmg, config_emulator_options.save_in_rom_folder);

                if (config_emulator_options.start_paused)
                {
                    emu->Pause();
                    
                    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
                        emu_frame_buffer[i] = 0;
                }
            }

            if (ImGui::MenuItem("Paused", "Ctrl+P", &config_emulator_options.paused))
            {
                if (emu->IsPaused())
                    emu->Resume();
                else
                    emu->Pause();
            }

            if (ImGui::MenuItem("Fast Forward", "Ctrl+F", &config_emulator_options.ffwd))
            {
                config_audio_options.sync = !config_emulator_options.ffwd;
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
           
            if (ImGui::BeginMenu("Select State Slot")) // <-- Append!
            {
                ImGui::Combo("", &config_emulator_options.save_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Save State", "Ctrl+S")) 
            {
                emu->SaveState(config_emulator_options.save_slot + 1);
            }

            if (ImGui::MenuItem("Load State", "Ctrl+L"))
            {
                emu->LoadState(config_emulator_options.save_slot + 1);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "Alt+F4"))
            {
                SDL_Event sdlevent;
                sdlevent.type = SDL_QUIT;
                SDL_PushEvent(&sdlevent);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Emulator"))
            {
                ImGui::MenuItem("Force DMG", "", &config_emulator_options.force_dmg);
                ImGui::MenuItem("Start Paused", "", &config_emulator_options.start_paused);
                ImGui::MenuItem("Save files in ROM folder", "", &config_emulator_options.save_in_rom_folder);
                
                if (ImGui::BeginMenu("Cheats"))
                {
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Video"))
            {
                ImGui::MenuItem("Show FPS", "", &config_video_options.fps);
                ImGui::MenuItem("Bilinear Filtering", "", &config_video_options.bilinear);
                ImGui::MenuItem("Screen Ghosting", "", &config_video_options.mix_frames, false);
                ImGui::MenuItem("Dot Matrix", "", &config_video_options.matrix, false);
                
                ImGui::Separator();

                if (ImGui::BeginMenu("Palette"))
                {
                    ImGui::Combo("", &config_emulator_options.save_slot, "Original\0Sharp\0Black & White\0Autumn\0Soft\0Slime\0Custom\0\0");
                    ImGui::EndMenu();
                }

                ImGui::ColorEdit4("Color #1", (float*)&config_video_options.color[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorEdit4("Color #2", (float*)&config_video_options.color[1], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorEdit4("Color #3", (float*)&config_video_options.color[2], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorEdit4("Color #4", (float*)&config_video_options.color[3], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Input"))
            {
                if (ImGui::BeginMenu("Keyboard Configuration"))
                {
                    ImGui::Text("Up:");
                    ImGui::SameLine();
                     if (ImGui::Button("UP"))
                        ImGui::OpenPopup("keyboard_definition");
                                        
                    ImGui::Text("Down:");
                    ImGui::SameLine();
                    if (ImGui::Button("DOWN"))
                        ImGui::OpenPopup("keyboard_definition");

                    if (ImGui::BeginPopupModal("keyboard_definition", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::Text("Press any key...\n\n");
                        ImGui::Separator();
                        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                        ImGui::SetItemDefaultFocus();
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                        ImGui::EndPopup();
                    }                   
                   
                    ImGui::EndMenu();
                }
                ImGui::MenuItem("Enable Gamepad", "", &config_input_options.gamepad, false);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Audio"))
            {
                if (ImGui::MenuItem("Enable", "", &config_audio_options.enable))
                {
                    emu->SetSoundSettings(config_audio_options.enable, 44100);
                }

                if (ImGui::MenuItem("Sync With Emulator", "", &config_audio_options.sync))
                {
                    config_emulator_options.ffwd = false;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Enabled", "", &gui_show_debug, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("About"))
        {
            ImGui::MenuItem("About " GEARBOY_TITLE " " GEARBOY_VERSION " ...", "", &gui_show_about_window);
            ImGui::EndMenu();
        }

        gui_main_menu_height = ImGui::GetWindowSize().y;

        ImGui::EndMainMenuBar();       
    }

    if (open_rom)
        ImGui::OpenPopup("Open ROM...");
    
    if (open_state)
        ImGui::OpenPopup("Load State From...");
    
    if (save_state)
        ImGui::OpenPopup("Save State As...");

    if(file_dialog.showFileDialog("Open ROM...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), "*.*,.gb,.gbc,.cgb,.sgb,.dmg,.rom,.zip"))
    {
        emu->Resume();
        emu->LoadRom(file_dialog.selected_path.c_str(), config_emulator_options.force_dmg, config_emulator_options.save_in_rom_folder);

        if (config_emulator_options.start_paused)
        {
            emu->Pause();
            
            for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
                emu_frame_buffer[i] = 0;
        }
    }

    if(file_dialog.showFileDialog("Load State From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".state,*.*"))
    {
        emu->LoadState(file_dialog.selected_path.c_str());
    }

    if(file_dialog.showFileDialog("Save State As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".state"))
    {
        std::string state_path = file_dialog.selected_path;

        if (state_path.rfind(file_dialog.ext) != (state_path.size()-file_dialog.ext.size()))
        {
            state_path += file_dialog.ext;
        }

        emu->SaveState(state_path.c_str());
    }
}

static void gui_main_window(void)
{
    int w = ImGui::GetIO().DisplaySize.x;
    int h = ImGui::GetIO().DisplaySize.y - gui_main_menu_height;

    int factor_w = w / GAMEBOY_WIDTH;
    int factor_h = h / GAMEBOY_HEIGHT;

    int factor = (factor_w < factor_h) ? factor_w : factor_h;

    gui_main_window_width = GAMEBOY_WIDTH * factor;
    gui_main_window_height = GAMEBOY_HEIGHT * factor;

    int window_x = (w - (GAMEBOY_WIDTH * factor)) / 2;
    int window_y = ((h - (GAMEBOY_HEIGHT * factor)) / 2) + gui_main_menu_height;
    
    ImGui::SetNextWindowPos(ImVec2(window_x, window_y));
    ImGui::SetNextWindowSize(ImVec2(gui_main_window_width, gui_main_window_height));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(GEARBOY_TITLE, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);

    ImGui::Image((void*)(intptr_t)emu_texture, ImVec2(gui_main_window_width,gui_main_window_height));

    if (config_video_options.fps)
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

static void gui_about_window(void)
{
    ImGui::SetNextWindowSize(ImVec2(400,250));
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 200, (ImGui::GetIO().DisplaySize.y / 2) - 115));

    ImGui::Begin("About " GEARBOY_TITLE, &gui_show_about_window, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    
    ImGui::Text("%s %s", GEARBOY_TITLE, GEARBOY_VERSION);
    ImGui::Text("Build: %s", EMULATOR_BUILD);
    ImGui::Separator();
    ImGui::Text("By Ignacio Sánchez (twitter.com/drhelius)");
    ImGui::Text("%s is licensed under the GPL-3.0 License,\nsee LICENSE for more information.", GEARBOY_TITLE);
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
#ifdef __GNUC__
    ImGui::Text("Built with GCC %d.", (int)__GNUC__);
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
    ImGui::End();
}
