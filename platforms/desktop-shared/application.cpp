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
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "emu.h"
#include "gui.h"
#include "gui_debug.h"
#include "config.h"
#include "renderer.h"
#include "utils.h"

#define APPLICATION_IMPORT
#include "application.h"

#if defined(GEARBOY_DEBUG)
#define WINDOW_TITLE GEARBOY_TITLE " " GEARBOY_VERSION " (DEBUG)"
#else
#define WINDOW_TITLE GEARBOY_TITLE " " GEARBOY_VERSION
#endif

static SDL_GLContext gl_context = NULL;
static bool running = true;
static bool paused_when_focus_lost = false;
static Uint64 frame_time_start = 0;
static Uint64 frame_time_end = 0;
static Uint32 mouse_last_motion_time = 0;
static const Uint32 mouse_hide_timeout_ms = 1500;

static bool sdl_init(void);
static void sdl_destroy(void);
static void sdl_load_gamepad_mappings(void);
static void sdl_events(void);
static void sdl_events_app(const SDL_Event* event);
static void sdl_events_emu(const SDL_Event* event);
static void sdl_shortcuts_gui(const SDL_Event* event);
static void sdl_add_gamepads(void);
static void sdl_remove_gamepad(SDL_JoystickID instance_id);
static void handle_mouse_cursor(void);
static void handle_menu(void);
static void run_emulator(void);
static void render(void);
static void frame_throttle(void);
static void save_window_size(void);
static void log_sdl_error(const char* action, const char* file, int line);

#define SDL_ERROR(action) log_sdl_error(action, __FILE__, __LINE__)

#if defined(__APPLE__)
#include <SDL_syswm.h>
static void* macos_fullscreen_observer = NULL;
static void* macos_nswindow = NULL;
extern "C" void* macos_install_fullscreen_observer(void* nswindow, void(*enter_cb)(), void(*exit_cb)());
extern "C" void macos_set_native_fullscreen(void* nswindow, bool enter);
#endif

int application_init(const char* rom_file, const char* symbol_file, bool force_fullscreen, bool force_windowed)
{
    Log("\n%s", GEARBOY_TITLE_ASCII);
    Log("%s %s Desktop App", GEARBOY_TITLE, GEARBOY_VERSION);

    config_init();
    config_read();

    application_show_menu = true;

    if (force_fullscreen)
    {
        config_emulator.fullscreen = true;
    }
    else if (force_windowed)
    {
        config_emulator.fullscreen = false;
    }

    if (!sdl_init())
    {
        Log("ERROR: Failed to initialize SDL");
        return 1;
    }

    strcpy(emu_savefiles_path, config_emulator.savefiles_path.c_str());
    strcpy(emu_savestates_path, config_emulator.savestates_path.c_str());
    emu_savefiles_dir_option = config_emulator.savefiles_dir_option;
    emu_savestates_dir_option = config_emulator.savestates_dir_option;

    if (!emu_init())
    {
        Log("ERROR: Failed to initialize emulator");
        return 2;
    }

    if (!gui_init())
    {
        Log("ERROR: Failed to initialize GUI");
        return 3;
    }

    if (!ImGui_ImplSDL2_InitForOpenGL(application_sdl_window, gl_context))
    {
        Log("ERROR: Failed to initialize ImGui for SDL2");
        return 4;
    }

    if (!renderer_init())
    {
        Log("ERROR: Failed to initialize renderer");
        return 5;
    }

    SDL_GL_SetSwapInterval(config_video.sync ? 1 : 0);

    if (config_emulator.fullscreen)
        application_trigger_fullscreen(true);

    if (IsValidPointer(rom_file) && (strlen(rom_file) > 0))
    {
        Log ("Rom file argument: %s", rom_file);
        gui_load_rom(rom_file);
    }
    if (IsValidPointer(symbol_file) && (strlen(symbol_file) > 0))
    {
        Log ("Symbol file argument: %s", symbol_file);
        gui_debug_reset_symbols();
        gui_debug_load_symbols_file(symbol_file);
    }

    return 0;
}

void application_destroy(void)
{
    save_window_size();
    config_write();
    emu_destroy();
    config_destroy();
    renderer_destroy();
    ImGui_ImplSDL2_Shutdown();
    gui_destroy();
    sdl_destroy();
}

void application_mainloop(void)
{
    Log("Starting main loop...");

    while (running)
    {
        frame_time_start = SDL_GetPerformanceCounter();
        sdl_events();
        handle_mouse_cursor();
        handle_menu();
        run_emulator();
        render();
        frame_time_end = SDL_GetPerformanceCounter();
        frame_throttle();
    }
}

void application_trigger_quit(void)
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

#if defined(__APPLE__)

static void on_enter_fullscreen()
{
    config_emulator.fullscreen = true;
}

static void on_exit_fullscreen()
{
    config_emulator.fullscreen = false;
}

#endif

void application_trigger_fullscreen(bool fullscreen)
{
#if defined(__APPLE__)
    macos_set_native_fullscreen(macos_nswindow, fullscreen);
#else
    SDL_SetWindowFullscreen(application_sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_ERROR("SDL_SetWindowFullscreen");
#endif
    mouse_last_motion_time = SDL_GetTicks();
}

void application_trigger_fit_to_content(int width, int height)
{
    SDL_SetWindowSize(application_sdl_window, width, height);
    SDL_ERROR("SDL_SetWindowSize");
}

void application_update_title_with_rom(const char* rom)
{
    char final_title[256];
    snprintf(final_title, 256, "%s - %s", WINDOW_TITLE, rom);
    SDL_SetWindowTitle(application_sdl_window, final_title);
    SDL_ERROR("SDL_SetWindowTitle");
}

void application_assign_gamepad(int device_index)
{
    if (device_index < 0)
    {
        if (IsValidPointer(application_gamepad))
        {
            SDL_GameControllerClose(application_gamepad);
            application_gamepad = NULL;
            Debug("Player controller set to None");
        }
        return;
    }

    if (device_index >= SDL_NumJoysticks())
    {
        Log("Warning: device_index %d out of range", device_index);
        return;
    }

    if (!SDL_IsGameController(device_index))
    {
        Log("Warning: device_index %d is not a game controller", device_index);
        return;
    }

    SDL_JoystickID wanted_id = SDL_JoystickGetDeviceInstanceID(device_index);

    if (IsValidPointer(application_gamepad))
    {
        SDL_JoystickID current_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(application_gamepad));
        if (current_id == wanted_id)
            return;
    }

    if (IsValidPointer(application_gamepad))
    {
        SDL_GameControllerClose(application_gamepad);
        application_gamepad = NULL;
    }

    SDL_GameController* controller = SDL_GameControllerOpen(device_index);
    if (!IsValidPointer(controller))
    {
        Log("SDL_GameControllerOpen failed for device_index %d", device_index);
        SDL_ERROR("SDL_GameControllerOpen");
        return;
    }

    application_gamepad = controller;
    Debug("Game controller %d assigned", device_index);
}

static bool sdl_init(void)
{
    Debug("Initializing SDL...");
    InitPointer(application_gamepad);

#if defined(_WIN32)
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
    SDL_ERROR("SDL_SetHint SDL_HINT_WINDOWS_DPI_SCALING");
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        SDL_ERROR("SDL_Init");
        return false;
    }

    SDL_VERSION(&application_sdl_build_version);
    SDL_GetVersion(&application_sdl_link_version);

    Log("Using SDL %d.%d.%d (build)", application_sdl_build_version.major, application_sdl_build_version.minor, application_sdl_build_version.patch);
    Log("Using SDL %d.%d.%d (link) ", application_sdl_link_version.major, application_sdl_link_version.minor, application_sdl_link_version.patch);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (config_emulator.maximized)
        window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_MAXIMIZED);

    application_sdl_window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config_emulator.window_width, config_emulator.window_height, window_flags);

    if (!application_sdl_window)
    {
        SDL_ERROR("SDL_CreateWindow");
        return false;
    }

    gl_context = SDL_GL_CreateContext(application_sdl_window);

    if (!gl_context)
    {
        SDL_ERROR("SDL_GL_CreateContext");
        return false;
    }

    SDL_GL_MakeCurrent(application_sdl_window, gl_context);
    SDL_ERROR("SDL_GL_MakeCurrent");

#if defined(__APPLE__)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(application_sdl_window, &info))
    {
        void* nswindow = info.info.cocoa.window;
        macos_nswindow = nswindow;
        macos_fullscreen_observer = macos_install_fullscreen_observer(nswindow, on_enter_fullscreen, on_exit_fullscreen);
    }
#endif

    SDL_GL_SetSwapInterval(0);
    SDL_ERROR("SDL_GL_SetSwapInterval");

    SDL_SetWindowMinimumSize(application_sdl_window, 500, 300);

    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(application_sdl_window, &w, &h);
    SDL_GL_GetDrawableSize(application_sdl_window, &display_w, &display_h);
    
    if (w > 0 && h > 0)
    {
        float scale_w = (float)display_w / w;
        float scale_h = (float)display_h / h;

        application_display_scale = (scale_w > scale_h) ? scale_w : scale_h;
    }

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_ERROR("SDL_EventState SDL_DROPFILE");

    sdl_load_gamepad_mappings();
    sdl_add_gamepads();

    return true;
}

static void sdl_destroy(void)
{
    SDL_GameControllerClose(application_gamepad);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(application_sdl_window);
    SDL_Quit();
}

static void sdl_load_gamepad_mappings(void)
{
    std::string db_path;
    char exe_path[1024] = { };
    get_executable_path(exe_path, sizeof(exe_path));

    if (exe_path[0] != '\0')
    {
        db_path = std::string(exe_path) + "/gamecontrollerdb.txt";
    }
    else
    {
        db_path = "gamecontrollerdb.txt";
    }

    std::ifstream file(db_path);
    if (!file.is_open())
    {
        file.open("gamecontrollerdb.txt");
    }

    int added_mappings = 0;
    int updated_mappings = 0;
    int line_number = 0;
    char platform_field[64] = { };
    snprintf(platform_field, 64, "platform:%s", SDL_GetPlatform());
    if (file.is_open())
    {
        Debug("Loading gamecontrollerdb.txt file");
        std::string line;
        while (std::getline(file, line))
        {
            size_t comment = line.find_first_of('#');
            if (comment != std::string::npos)
                line = line.substr(0, comment);
            line = line.erase(0, line.find_first_not_of(" \t\r\n"));
            line = line.erase(line.find_last_not_of(" \t\r\n") + 1);
            while (line[0] == ' ')
                line = line.substr(1);
            if (line.empty())
                continue;
            if ((line.find("platform:") != std::string::npos) && (line.find(platform_field) == std::string::npos))
                continue;
            int result = SDL_GameControllerAddMapping(line.c_str());
            if (result == 1)
                added_mappings++;
            else if (result == 0)
                updated_mappings++;
            else if (result == -1)
            {
                Log("ERROR: Unable to load game controller mapping in line %d from gamecontrollerdb.txt", line_number);
                SDL_ERROR("SDL_GameControllerAddMapping");
            }
            line_number++;
        }
        file.close();
    }
    else
    {
        Log("ERROR: Game controller database not found (gamecontrollerdb.txt)!!");
        return;
    }
    Log("Added %d new game controller mappings from gamecontrollerdb.txt", added_mappings);
    Log("Updated %d game controller mappings from gamecontrollerdb.txt", updated_mappings);
    application_added_gamepad_mappings = added_mappings;
    application_updated_gamepad_mappings = updated_mappings;
}

static void handle_mouse_cursor(void)
{
    if (!config_debug.debug && gui_main_window_hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    else if (!config_debug.debug && config_emulator.fullscreen && !config_emulator.always_show_menu)
    {
        Uint32 now = SDL_GetTicks();

        if ((now - mouse_last_motion_time) < mouse_hide_timeout_ms)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        else
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    }
    else
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
}

static void handle_menu(void)
{
    if (config_emulator.always_show_menu)
        application_show_menu = true;
    else if (config_debug.debug)
        application_show_menu = true;
    else if (config_emulator.fullscreen)
        application_show_menu = config_emulator.always_show_menu;
    else
        application_show_menu = true;
}

static void sdl_events(void)
{
    SDL_Event event;
        
    while (SDL_PollEvent(&event))
    {
        sdl_events_app(&event);

        if (running)
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (!gui_in_use)
            {
                sdl_events_emu(&event);
                sdl_shortcuts_gui(&event);
            }
        }
    }
}

static void sdl_events_app(const SDL_Event* event)
{
    if (event->type == SDL_QUIT)
    {
        running = false;
        return;
    }

    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_CLOSE && event->window.windowID == SDL_GetWindowID(application_sdl_window))
    {
        running = false;
        return;
    }

    switch (event->type)
    {
        case SDL_CONTROLLERDEVICEADDED:
        {
            sdl_add_gamepads();
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED:
        {
            sdl_remove_gamepad(event->cdevice.which);
            break;
        }
    }
}

static void sdl_events_emu(const SDL_Event* event)
{
    switch(event->type)
    {
        case (SDL_DROPFILE):
        {
            char* dropped_filedir = event->drop.file;
            gui_load_rom(dropped_filedir);
            SDL_free(dropped_filedir);
            SDL_SetWindowInputFocus(application_sdl_window);
        }
        break;

        case SDL_WINDOWEVENT:
        {
            switch (event->window.event)
            {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    if (!paused_when_focus_lost)
                        emu_resume();
                }
                break;

                case SDL_WINDOWEVENT_FOCUS_LOST:
                {
                    paused_when_focus_lost = emu_is_paused();
                    emu_pause();
                }
                break;
            }
        }
        break;

        case (SDL_MOUSEMOTION):
        {
            mouse_last_motion_time = SDL_GetTicks();
        }
        break;

        case SDL_CONTROLLERBUTTONDOWN:
        {
            if (!IsValidPointer(application_gamepad))
                break;

            SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(application_gamepad));

            if (!config_input.gamepad)
                break;

            if (event->cbutton.which != id)
                break;

            if (event->cbutton.button == config_input.gamepad_b)
                emu_key_pressed(B_Key);
            else if (event->cbutton.button == config_input.gamepad_a)
                emu_key_pressed(A_Key);
            else if (event->cbutton.button == config_input.gamepad_select)
                emu_key_pressed(Select_Key);
            else if (event->cbutton.button == config_input.gamepad_start)
                emu_key_pressed(Start_Key);

            if (config_input.gamepad_directional == 1)
                break;

            if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
                emu_key_pressed(Up_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                emu_key_pressed(Down_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                emu_key_pressed(Left_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                emu_key_pressed(Right_Key);
        }
        break;

        case SDL_CONTROLLERBUTTONUP:
        {
            if (!IsValidPointer(application_gamepad))
                break;

            SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(application_gamepad));

            if (!config_input.gamepad)
                break;

            if (event->cbutton.which != id)
                break;

            if (event->cbutton.button == config_input.gamepad_b)
                emu_key_released(B_Key);
            else if (event->cbutton.button == config_input.gamepad_a)
                emu_key_released(A_Key);
            else if (event->cbutton.button == config_input.gamepad_select)
                emu_key_released(Select_Key);
            else if (event->cbutton.button == config_input.gamepad_start)
                emu_key_released(Start_Key);

            if (config_input.gamepad_directional == 1)
                break;

            if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
                emu_key_released(Up_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                emu_key_released(Down_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                emu_key_released(Left_Key);
            else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                emu_key_released(Right_Key);
        }
        break;

        case SDL_CONTROLLERAXISMOTION:
        {
            if (!IsValidPointer(application_gamepad))
                break;

            SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(application_gamepad));

            if (!config_input.gamepad)
                break;

            if (event->caxis.which != id)
                break;

            if (config_input.gamepad_directional == 1)
            {
                const int STICK_DEAD_ZONE = 8000;

                if(event->caxis.axis == config_input.gamepad_x_axis)
                {
                    int x_motion = event->caxis.value * (config_input.gamepad_invert_x_axis ? -1 : 1);

                    if (x_motion < -STICK_DEAD_ZONE)
                        emu_key_pressed(Left_Key);
                    else if (x_motion > STICK_DEAD_ZONE)
                        emu_key_pressed(Right_Key);
                    else
                    {
                        emu_key_released(Left_Key);
                        emu_key_released(Right_Key);
                    }
                }
                else if(event->caxis.axis == config_input.gamepad_y_axis)
                {
                    int y_motion = event->caxis.value * (config_input.gamepad_invert_y_axis ? -1 : 1);

                    if (y_motion < -STICK_DEAD_ZONE)
                        emu_key_pressed(Up_Key);
                    else if (y_motion > STICK_DEAD_ZONE)
                        emu_key_pressed(Down_Key);
                    else
                    {
                        emu_key_released(Up_Key);
                        emu_key_released(Down_Key);
                    }
                }
            }

            if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
            {
                int vbtn = GAMEPAD_VBTN_AXIS_BASE + event->caxis.axis;
                bool pressed = event->caxis.value > GAMEPAD_VBTN_AXIS_THRESHOLD;

                if (config_input.gamepad_a == vbtn)
                {
                    if (pressed)
                        emu_key_pressed(A_Key);
                    else
                        emu_key_released(A_Key);
                }
                if (config_input.gamepad_b == vbtn)
                {
                    if (pressed)
                        emu_key_pressed(B_Key);
                    else
                        emu_key_released(B_Key);
                }
                if (config_input.gamepad_start == vbtn)
                {
                    if (pressed)
                        emu_key_pressed(Start_Key);
                    else
                        emu_key_released(Start_Key);
                }
                if (config_input.gamepad_select == vbtn)
                {
                    if (pressed)
                        emu_key_pressed(Select_Key);
                    else
                        emu_key_released(Select_Key);
                }
            }
        }
        break;

        case SDL_KEYDOWN:
        {
            if (event->key.repeat != 0)
                break;

            if (event->key.keysym.mod & KMOD_CTRL)
                break;

            int key = event->key.keysym.scancode;

            if (key == SDL_SCANCODE_ESCAPE)
            {
                config_emulator.fullscreen = false;
                application_trigger_fullscreen(false);
                break;
            }

            if (key == SDL_SCANCODE_F11)
            {
                config_emulator.fullscreen = !config_emulator.fullscreen;
                application_trigger_fullscreen(config_emulator.fullscreen);
                break;
            }

            if (key == config_input.key_left)
                emu_key_pressed(Left_Key);
            else if (key == config_input.key_right)
                emu_key_pressed(Right_Key);
            else if (key == config_input.key_up)
                emu_key_pressed(Up_Key);
            else if (key == config_input.key_down)
                emu_key_pressed(Down_Key);
            else if (key == config_input.key_b)
                emu_key_pressed(B_Key);
            else if (key == config_input.key_a)
                emu_key_pressed(A_Key);
            else if (key == config_input.key_select)
                emu_key_pressed(Select_Key);
            else if (key == config_input.key_start)
                emu_key_pressed(Start_Key);
        }
        break;

        case SDL_KEYUP:
        {
            int key = event->key.keysym.scancode;

            if (key == config_input.key_left)
                emu_key_released(Left_Key);
            else if (key == config_input.key_right)
                emu_key_released(Right_Key);
            else if (key == config_input.key_up)
                emu_key_released(Up_Key);
            else if (key == config_input.key_down)
                emu_key_released(Down_Key);
            else if (key == config_input.key_b)
                emu_key_released(B_Key);
            else if (key == config_input.key_a)
                emu_key_released(A_Key);
            else if (key == config_input.key_select)
                emu_key_released(Select_Key);
            else if (key == config_input.key_start)
                emu_key_released(Start_Key);
        }
        break;
    }
}

static void sdl_shortcuts_gui(const SDL_Event* event)
{
    if ((event->type == SDL_KEYDOWN) && (event->key.keysym.mod & KMOD_CTRL))
    {
        int key = event->key.keysym.scancode;

        switch (key)
        {
            case SDL_SCANCODE_Q:
                application_trigger_quit();
                break;
            case SDL_SCANCODE_C:
                gui_shortcut(gui_ShortcutDebugCopy);
                break;
            case SDL_SCANCODE_V:
                gui_shortcut(gui_ShortcutDebugPaste);
                break;
            case SDL_SCANCODE_O:
                gui_shortcut(gui_ShortcutOpenROM);
                break;
            case SDL_SCANCODE_R:
                gui_shortcut(gui_ShortcutReset);
                break;
            case SDL_SCANCODE_P:
                gui_shortcut(gui_ShortcutPause);
                break;
            case SDL_SCANCODE_F:
                gui_shortcut(gui_ShortcutFFWD);
                break;
            case SDL_SCANCODE_L:
                gui_shortcut(gui_ShortcutLoadState);
                break;
            case SDL_SCANCODE_S:
                gui_shortcut(gui_ShortcutSaveState);
                break;
            case SDL_SCANCODE_X:
                gui_shortcut(gui_ShortcutScreenshot);
                break;
            case SDL_SCANCODE_M:
                gui_shortcut(gui_ShortcutShowMainMenu);
                break;
            case SDL_SCANCODE_F5:
                gui_shortcut(gui_ShortcutDebugContinue);
                break;
            case SDL_SCANCODE_F6:
                gui_shortcut(gui_ShortcutDebugNextFrame);
                break;
            case SDL_SCANCODE_F8:
                gui_shortcut(gui_ShortcutDebugRuntocursor);
                break;
            case SDL_SCANCODE_F9:
                gui_shortcut(gui_ShortcutDebugBreakpoint);
                break;
            case SDL_SCANCODE_F10:
                gui_shortcut(gui_ShortcutDebugStep);
                break;
            case SDL_SCANCODE_BACKSPACE:
                gui_shortcut(gui_ShortcutDebugGoBack);
                break;
        }
    }
}

static void sdl_add_gamepads(void)
{
    if (IsValidPointer(application_gamepad))
    {
        SDL_Joystick* js = SDL_GameControllerGetJoystick(application_gamepad);

        if (!IsValidPointer(js) || SDL_JoystickGetAttached(js) == SDL_FALSE)
        {
            SDL_GameControllerClose(application_gamepad);
            application_gamepad = NULL;
            Debug("Game controller closed when adding a new gamepad");
        }
    }

    bool connected = IsValidPointer(application_gamepad);

    if (connected)
        return;

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (!SDL_IsGameController(i))
            continue;

        SDL_GameController* controller = SDL_GameControllerOpen(i);
        if (!IsValidPointer(controller))
        {
            Log("Warning: Unable to open game controller %d!\n", i);
            SDL_ERROR("SDL_GameControllerOpen");
            continue;
        }

        if (!connected)
        {
            application_gamepad = controller;
            connected = true;
            Debug("Game controller %d assigned to Player 1", i);
        }
        else
        {
            SDL_GameControllerClose(controller);
            Debug("Game controller %d detected but all player slots are full", i);
        }

        if (connected)
            break;
    }
}

static void sdl_remove_gamepad(SDL_JoystickID instance_id)
{
    if (application_gamepad != NULL)
    {
        SDL_JoystickID current_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(application_gamepad));
        if (current_id == instance_id)
        {
            SDL_GameControllerClose(application_gamepad);
            application_gamepad = NULL;
            Debug("Game controller %d disconnected", instance_id);
        }
    }
}

static void run_emulator(void)
{
    config_emulator.paused = emu_is_paused();
    emu_audio_sync = config_audio.sync;
    emu_update();
}

static void render(void)
{
    renderer_begin_render();
    ImGui_ImplSDL2_NewFrame();
    gui_render();
    renderer_render();
    renderer_end_render();

    SDL_GL_SwapWindow(application_sdl_window);
}

static void frame_throttle(void)
{
    if (emu_is_empty() || emu_is_paused() || !emu_is_audio_open() || config_emulator.ffwd)
    {
        Uint64 count_per_sec = SDL_GetPerformanceFrequency();
        float elapsed = (float)(frame_time_end - frame_time_start) / (float)count_per_sec;
        elapsed *= 1000.0f;

        float min = 16.666f;

        if (config_emulator.ffwd)
        {
            switch (config_emulator.ffwd_speed)
            {
                case 0:
                    min = 16.666f / 1.5f;
                    break;
                case 1: 
                    min = 16.666f / 2.0f;
                    break;
                case 2:
                    min = 16.666f / 2.5f;
                    break;
                case 3:
                    min = 16.666f / 3.0f;
                    break;
                default:
                    min = 0.0f;
            }
        }

        if (elapsed < min)
            SDL_Delay((Uint32)(min - elapsed));
    }
}

static void save_window_size(void)
{
    if (!config_emulator.fullscreen)
    {
        int width, height;
        SDL_GetWindowSize(application_sdl_window, &width, &height);
        config_emulator.window_width = width;
        config_emulator.window_height = height;
        config_emulator.maximized = (SDL_GetWindowFlags(application_sdl_window) & SDL_WINDOW_MAXIMIZED);
    }
}

static void log_sdl_error(const char* action, const char* file, int line)
{
    const char* error = SDL_GetError();
    if (error && error[0] != '\0')
    {
        Log("SDL Error: %s (%s:%d) - %s", action, file, line, error);
        SDL_ClearError();
    }
}
