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
#include "ogl_renderer.h"
#include "emu.h"
#include "application.h"

#define DISPLAY_IMPORT
#include "display.h"

static Uint64 frame_time_start = 0;
static Uint64 frame_time_end = 0;
static double monitor_refresh_rate = 60.0;
static double content_frame_rate = 60.0;
static double vsync_frame_accumulator = 0.0;
static int last_vsync_state = -1;
static bool multi_monitor_mixed_refresh = false;
static bool last_link_cable_connected = false;
static bool fixed_vsync_fallback_logged = false;
static bool pending_gl_context_recreate = false;

static bool display_is_vrr_enabled(void);
static bool display_fixed_vsync_supported(void);
static bool display_update_content_frame_rate(void);
static void display_reset_vsync_accumulator(void);
static void display_set_swap_interval(bool enabled);

void display_begin_frame(void)
{
    frame_time_start = SDL_GetPerformanceCounter();
}

void display_render(void)
{
    ogl_renderer_begin_render();
    ImGui_ImplSDL3_NewFrame();
    gui_render();
    ogl_renderer_render();
    ogl_renderer_end_render();

    SDL_GL_SwapWindow(application_sdl_window);
}

void display_frame_throttle(void)
{
    frame_time_end = SDL_GetPerformanceCounter();

    if (emu_is_empty() || emu_is_paused() || emu_is_debug_idle() || !emu_is_audio_open() || config_emulator.ffwd)
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

bool display_should_run_emu_frame(void)
{
    if (display_is_vrr_enabled())
        return true;

    if (display_update_content_frame_rate() && config_video.sync_mode == config_VideoSync_Fixed)
        display_use_vsync_if_enabled();

    if (config_video.sync_mode == config_VideoSync_Fixed && last_vsync_state == 1
        && !emu_is_empty() && !emu_is_paused()
        && !emu_is_debug_idle() && emu_is_audio_open() && !config_emulator.ffwd
        && !emu_link_cable_is_cable_connected())
    {
        if (!display_fixed_vsync_supported() || content_frame_rate + 0.000001 >= monitor_refresh_rate)
            return true;

        vsync_frame_accumulator += content_frame_rate;

        if (vsync_frame_accumulator + 0.000001 >= monitor_refresh_rate)
        {
            vsync_frame_accumulator -= monitor_refresh_rate;
            if (vsync_frame_accumulator < 0.0)
                vsync_frame_accumulator = 0.0;
            return true;
        }

        return false;
    }

    return true;
}

void display_use_vsync_if_enabled(void)
{
    display_update_frame_pacing();

    bool effective = config_video.sync_mode != config_VideoSync_Disabled &&
        !display_is_vsync_forced_off() &&
        !emu_link_cable_is_cable_connected();

    if (config_video.sync_mode == config_VideoSync_Fixed && !display_fixed_vsync_supported())
        effective = false;

    display_set_swap_interval(effective);
}

void display_disable_vsync(void)
{
    display_set_swap_interval(false);
    display_update_frame_pacing();
}

void display_update_vsync_state(void)
{
    bool connected = emu_link_cable_is_cable_connected();

    if (connected == last_link_cable_connected)
        return;

    last_link_cable_connected = connected;
    display_use_vsync_if_enabled();
}

void display_update_frame_pacing(void)
{
    SDL_DisplayID display = SDL_GetDisplayForWindow(application_sdl_window);

    if (display == 0)
        display = SDL_GetPrimaryDisplay();

    double refresh_rate = 60.0;
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);

    if (mode)
    {
        if (mode->refresh_rate_numerator > 0 && mode->refresh_rate_denominator > 0)
            refresh_rate = (double)mode->refresh_rate_numerator / (double)mode->refresh_rate_denominator;
        else if (mode->refresh_rate > 0.0f)
            refresh_rate = (double)mode->refresh_rate;
    }

    if (refresh_rate < monitor_refresh_rate - 0.000001 || refresh_rate > monitor_refresh_rate + 0.000001)
    {
        monitor_refresh_rate = refresh_rate;
        display_reset_vsync_accumulator();
    }

    display_update_content_frame_rate();

    if (config_video.sync_mode == config_VideoSync_Fixed && last_vsync_state == 1 && !display_fixed_vsync_supported())
        display_set_swap_interval(false);

    Debug("Monitor refresh rate: %.3f Hz, content frame rate: %.3f FPS%s",
        monitor_refresh_rate, content_frame_rate, display_is_vrr_enabled() ? " (VRR)" : "");
}

void display_check_mixed_refresh_rates(void)
{
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);

    if (!displays || count <= 1)
    {
        if (displays)
            SDL_free(displays);
        multi_monitor_mixed_refresh = false;
        return;
    }

    int first_rate = 0;
    bool mixed = false;

    for (int i = 0; i < count; i++)
    {
        const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displays[i]);
        if (mode && mode->refresh_rate > 0)
        {
            int rate = (int)mode->refresh_rate;
            if (first_rate == 0)
                first_rate = rate;
            else if (rate != first_rate)
            {
                mixed = true;
                break;
            }
        }
    }

    SDL_free(displays);

    if (mixed != multi_monitor_mixed_refresh)
    {
        multi_monitor_mixed_refresh = mixed;
        if (mixed)
            Log("Multiple monitors with different refresh rates detected");

        if (display_is_vsync_forced_off())
        {
            display_set_swap_interval(false);
            Debug("Vsync forced off: multi-viewport with mixed refresh rate monitors");
        }
        else if (config_video.sync_mode != config_VideoSync_Disabled)
        {
            display_use_vsync_if_enabled();
        }
    }
}

bool display_is_vsync_forced_off(void)
{
    return config_debug.debug && config_debug.multi_viewport && multi_monitor_mixed_refresh;
}

void display_request_gl_context_recreate(void)
{
    pending_gl_context_recreate = true;
}

void display_recreate_gl_context(void)
{
    ogl_renderer_destroy();
    ImGui_ImplSDL3_Shutdown();

    SDL_GLContext old_context = display_gl_context;
    display_gl_context = SDL_GL_CreateContext(application_sdl_window);

    if (display_gl_context)
    {
        SDL_GL_MakeCurrent(application_sdl_window, display_gl_context);
        SDL_GL_DestroyContext(old_context);

        last_vsync_state = -1;
        display_use_vsync_if_enabled();

        ImGui_ImplSDL3_InitForOpenGL(application_sdl_window, display_gl_context);
        ogl_renderer_init();
    }
}

static bool display_is_vrr_enabled(void)
{
#if defined(_WIN32)
    return config_video.sync_mode == config_VideoSync_VRR;
#else
    return false;
#endif
}

static bool display_fixed_vsync_supported(void)
{
    bool supported = monitor_refresh_rate + 0.1 >= content_frame_rate;

    if (!supported && !fixed_vsync_fallback_logged)
    {
        Log("Fixed VSync disabled: %.3f Hz monitor is slower than %.3f FPS content",
            monitor_refresh_rate, content_frame_rate);
        fixed_vsync_fallback_logged = true;
    }
    else if (supported)
        fixed_vsync_fallback_logged = false;

    return supported;
}

static bool display_update_content_frame_rate(void)
{
    double frame_rate = emu_get_frame_rate();

    if (frame_rate <= 0.0)
        frame_rate = 60.0;

    if (frame_rate >= content_frame_rate - 0.000001 && frame_rate <= content_frame_rate + 0.000001)
        return false;

    content_frame_rate = frame_rate;
    display_reset_vsync_accumulator();
    return true;
}

static void display_reset_vsync_accumulator(void)
{
    vsync_frame_accumulator = monitor_refresh_rate - content_frame_rate;
    if (vsync_frame_accumulator < 0.0)
        vsync_frame_accumulator = 0.0;
}

static void display_set_swap_interval(bool enabled)
{
    int requested = enabled ? 1 : 0;

    if (!SDL_GL_SetSwapInterval(requested))
    {
        Log("SDL_GL_SetSwapInterval(%d) failed: %s", requested, SDL_GetError());
        last_vsync_state = -1;
        return;
    }

    int effective = 0;
    if (!SDL_GL_GetSwapInterval(&effective))
    {
        Log("SDL_GL_GetSwapInterval failed: %s", SDL_GetError());
        last_vsync_state = -1;
        return;
    }

    if (effective != last_vsync_state)
    {
        Debug("Swap interval: %d", effective);
        if (effective == 1)
            display_reset_vsync_accumulator();
    }

    last_vsync_state = effective;
}
