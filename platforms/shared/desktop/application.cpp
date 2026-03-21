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
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "gearboy.h"
#include "config.h"
#include "gui.h"
#include "gui_filedialogs.h"
// #include "gui_debug_disassembler.h"
#include "ogl_renderer.h"
#include "emu.h"
#include "display.h"
#include "utils.h"
#include "single_instance.h"
#include "gamepad.h"
#include "events.h"

#define APPLICATION_IMPORT
#include "application.h"

#if defined(DEBUG_GEARBOY)
#define WINDOW_TITLE GEARBOY_TITLE " " GEARBOY_VERSION " (DEBUG)"
#else
#define WINDOW_TITLE GEARBOY_TITLE " " GEARBOY_VERSION
#endif

static bool running = true;
static bool paused_when_focus_lost = false;
static Uint64 mouse_last_motion_time = 0;
static const Uint64 mouse_hide_timeout_ms = 1500;
static SDL_DisplayID current_display_id = 0;

// bool g_mcp_stdio_mode = false;

static bool sdl_init(void);
static void sdl_destroy(void);
static void sdl_events(void);
static void sdl_events_quit(const SDL_Event* event);
static void sdl_events_app(const SDL_Event* event);
static void handle_mouse_cursor(void);
static void handle_menu(void);
static void handle_single_instance(void);
static void run_emulator(void);
static void save_window_size(void);

#if defined(__APPLE__)
static void* macos_fullscreen_observer = NULL;
static void* macos_nswindow = NULL;
extern "C" void* macos_install_fullscreen_observer(void* nswindow, void(*enter_cb)(), void(*exit_cb)());
extern "C" void macos_set_native_fullscreen(void* nswindow, bool enter);
#endif

int application_init(const char* rom_file, const char* symbol_file, bool force_fullscreen, bool force_windowed, int mcp_mode, int mcp_tcp_port)
{
    (void)mcp_mode;
    (void)mcp_tcp_port;
    Log("\n%s", GEARBOY_TITLE_ASCII);
    Log("%s %s Desktop App", GEARBOY_TITLE, GEARBOY_VERSION);

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
        Error("Failed to initialize SDL");
        return 1;
    }

    if (!gamepad_init())
    {
        Error("Failed to initialize gamepad subsystem");
        return 2;
    }

    if (!emu_init())
    {
        Error("Failed to initialize emulator");
        return 3;
    }

    if (!gui_init())
    {
        Error("Failed to initialize GUI");
        return 4;
    }

    if (!ImGui_ImplSDL3_InitForOpenGL(application_sdl_window, display_gl_context))
    {
        Error("Failed to initialize ImGui for SDL3");
        return 5;
    }

    if (!ogl_renderer_init())
    {
        Error("Failed to initialize renderer");
        return 6;
    }

    if (config_emulator.fullscreen)
        application_trigger_fullscreen(true);

    if (IsValidPointer(rom_file) && (strlen(rom_file) > 0))
    {
        Log("Rom file argument: %s", rom_file);
        gui_load_rom(rom_file);
    }

    if (IsValidPointer(symbol_file) && (strlen(symbol_file) > 0))
    {
        Log("Symbol file argument: %s", symbol_file);
        // gui_debug_reset_symbols();
        // gui_debug_load_symbols_file(symbol_file);
    }

    // if (mcp_mode >= 0)
    // {
    //     Log("Auto-starting MCP server (mode: %s, port: %d)...", 
    //         mcp_mode == 0 ? "stdio" : "http", mcp_tcp_port);
    //     config_debug.debug = true;
    //     emu_set_overscan(0);
    //     emu_mcp_set_transport(mcp_mode, mcp_tcp_port);
    //     emu_mcp_start();
    // }

    return 0;
}

void application_destroy(void)
{
    save_window_size();
    emu_destroy();
    ogl_renderer_destroy();
    ImGui_ImplSDL3_Shutdown();
    gui_destroy();
    gamepad_destroy();
    sdl_destroy();
    single_instance_destroy();
}

void application_mainloop(void)
{
    Log("Running main loop...");

    while (running)
    {
        display_begin_frame();
        sdl_events();
        handle_mouse_cursor();
        handle_menu();
        handle_single_instance();
        run_emulator();
        display_render();
        display_frame_throttle();
    }
}

void application_trigger_quit(void)
{
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_EVENT_QUIT;
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
    if (fullscreen)
    {
        if (config_emulator.fullscreen_mode == 1)
        {
            // Exclusive fullscreen: set a display mode
            SDL_DisplayID display = SDL_GetDisplayForWindow(application_sdl_window);
            const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
            SDL_SetWindowFullscreenMode(application_sdl_window, mode);
        }
        else
        {
            // Desktop fullscreen: no display mode
            SDL_SetWindowFullscreenMode(application_sdl_window, NULL);
        }
        SDL_SetWindowFullscreen(application_sdl_window, true);
    }
    else
    {
        SDL_SetWindowFullscreen(application_sdl_window, false);
    }
    config_emulator.fullscreen = fullscreen;
#endif

    mouse_last_motion_time = SDL_GetTicks();
    display_update_frame_pacing();
}

void application_trigger_fit_to_content(int width, int height)
{
    SDL_SetWindowSize(application_sdl_window, width, height);
}


void application_update_title_with_rom(const char* rom)
{
    char final_title[256];
    snprintf(final_title, 256, "%s - %s", WINDOW_TITLE, rom);
    SDL_SetWindowTitle(application_sdl_window, final_title);
}

void application_input_pump(void)
{
    events_emu();
}

bool application_check_single_instance(const char* rom_file, const char* symbol_file)
{
    if (!config_debug.single_instance)
        return true;

    single_instance_init(GEARBOY_TITLE);

    if (!single_instance_try_lock())
    {
        if (rom_file != NULL || symbol_file != NULL)
            single_instance_send_message(rom_file, symbol_file);

        single_instance_destroy();
        return false;
    }

    return true;
}

static bool sdl_init(void)
{
    Debug("Initializing SDL...");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        SDL_ERROR("SDL_Init");
        return false;
    }

    int sdl_version = SDL_GetVersion();
    application_sdl_version_major = SDL_VERSIONNUM_MAJOR(sdl_version);
    application_sdl_version_minor = SDL_VERSIONNUM_MINOR(sdl_version);
    application_sdl_version_patch = SDL_VERSIONNUM_MICRO(sdl_version);

    Log("Using SDL %d.%d.%d", application_sdl_version_major, application_sdl_version_minor, application_sdl_version_patch);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#if defined(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE)
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
#endif
#if defined(__APPLE__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (config_emulator.maximized)
        window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_MAXIMIZED);

    float content_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (content_scale <= 0.0f)
        content_scale = 1.0f;

    application_sdl_window = SDL_CreateWindow(WINDOW_TITLE, (int)(config_emulator.window_width * content_scale), (int)(config_emulator.window_height * content_scale), window_flags);

    if (!application_sdl_window)
    {
        SDL_ERROR("SDL_CreateWindow");
        return false;
    }

    SDL_SetWindowPosition(application_sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    display_gl_context = SDL_GL_CreateContext(application_sdl_window);

    if (!display_gl_context)
    {
        SDL_ERROR("SDL_GL_CreateContext");
        return false;
    }

    SDL_GL_MakeCurrent(application_sdl_window, display_gl_context);
    SDL_ERROR("SDL_GL_MakeCurrent");

#if defined(__APPLE__)
    void* nswindow = (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(application_sdl_window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    if (nswindow)
    {
        macos_nswindow = nswindow;
        macos_fullscreen_observer = macos_install_fullscreen_observer(nswindow, on_enter_fullscreen, on_exit_fullscreen);
    }
#endif

    display_set_vsync(config_video.sync);
    display_check_mixed_refresh_rates();

    SDL_SetWindowMinimumSize(application_sdl_window, (int)(500 * content_scale), (int)(300 * content_scale));

    float display_scale = SDL_GetWindowDisplayScale(application_sdl_window);
    Log("Display scale: %.2f", display_scale);

    return true;
}

static void sdl_destroy(void)
{
    SDL_GL_DestroyContext(display_gl_context);
    SDL_DestroyWindow(application_sdl_window);
    SDL_Quit();
}

static void handle_mouse_cursor(void)
{
    if (!config_debug.debug && gui_main_window_hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    else if (!config_debug.debug && config_emulator.fullscreen && !config_emulator.always_show_menu)
    {
        Uint64 now = SDL_GetTicks();

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

static void handle_single_instance(void)
{
    if (!config_debug.single_instance || !single_instance_is_primary())
        return;

    single_instance_poll();

    static char s_pending_rom_path[4096];
    static char s_pending_symbol_path[4096];
    if (single_instance_get_pending_load(s_pending_rom_path, sizeof(s_pending_rom_path), s_pending_symbol_path, sizeof(s_pending_symbol_path)))
    {
        if (s_pending_rom_path[0] != '\0')
            gui_load_rom(s_pending_rom_path);
        if (s_pending_symbol_path[0] != '\0')
        {
            // gui_debug_reset_symbols();
            // gui_debug_load_symbols_file(s_pending_symbol_path);
        }
    }
}

static void sdl_events(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        bool file_dialog_active = gui_file_dialog_is_active();

        sdl_events_quit(&event);

        if (running)
        {
            sdl_events_app(&event);

            if (!file_dialog_active)
                ImGui_ImplSDL3_ProcessEvent(&event);

            if (!gui_in_use && !file_dialog_active)
            {
                events_handle_emu_event(&event);
                events_shortcuts(&event);
            }
        }
    }
}

static void sdl_events_quit(const SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        running = false;
        return;
    }

    if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(application_sdl_window))
    {
        running = false;
        return;
    }
}

static void sdl_events_app(const SDL_Event* event)
{
    switch (event->type)
    {
        case (SDL_EVENT_DROP_FILE):
        {
            const char* dropped_filedir = event->drop.data;
            gui_load_rom(dropped_filedir);
            SDL_RaiseWindow(application_sdl_window);
            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            display_set_vsync(config_video.sync);
            if (config_emulator.pause_when_inactive && !paused_when_focus_lost)
                emu_resume();
            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            display_set_vsync(false);
            if (config_emulator.pause_when_inactive)
            {
                paused_when_focus_lost = emu_is_paused();
                emu_pause();
            }
            break;
        }
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        {
            SDL_DisplayID new_display = SDL_GetDisplayForWindow(application_sdl_window);
            if (new_display != current_display_id)
            {
                current_display_id = new_display;
                display_check_mixed_refresh_rates();
                if (config_video.sync && !display_is_vsync_forced_off())
                    display_recreate_gl_context();
                else
                {
                    display_request_gl_context_recreate();
                    display_update_frame_pacing();
                }
            }
            break;
        }
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
        {
            display_check_mixed_refresh_rates();
            break;
        }
        case (SDL_EVENT_MOUSE_MOTION):
        {
            mouse_last_motion_time = SDL_GetTicks();
            break;
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        {
            gamepad_add();
            break;
        }
        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            gamepad_remove(event->gdevice.which);
            break;
        }
    }
}

static void run_emulator(void)
{
    if (!display_should_run_emu_frame())
        return;

    config_emulator.paused = emu_is_paused();
    emu_audio_sync = config_audio.sync;
    emu_update();

    if (!events_input_updated())
        events_emu();
    events_reset_input();
}

static void save_window_size(void)
{
    if (!config_emulator.fullscreen)
    {
        int width, height;
        SDL_GetWindowSize(application_sdl_window, &width, &height);
        float content_scale = SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(application_sdl_window));
        if (content_scale <= 0.0f)
            content_scale = 1.0f;
        config_emulator.window_width = (int)(width / content_scale);
        config_emulator.window_height = (int)(height / content_scale);
        config_emulator.maximized = (SDL_GetWindowFlags(application_sdl_window) & SDL_WINDOW_MAXIMIZED);
    }
}