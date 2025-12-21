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

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl.h>
#else
    #define GLAD_GL_IMPLEMENTATION
    #include "glad/glad.h"
    #include <SDL_opengl.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "emu.h"
#include "config.h"
#include "../../src/gearboy.h"

#define RENDERER_IMPORT
#include "renderer.h"

static uint32_t gameboy_texture;
static uint32_t matrix_texture;
static uint32_t frame_buffer_object;
static bool first_frame;
static u32 matrix[16] = {0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
static const int FRAME_BUFFER_SCALE = 4;

static void init_ogl_gui(void);
static void init_ogl_emu(void);
static void init_ogl_debug(void);
static void init_matrix_textures(void);
static void render_gui(void);
static void render_emu_normal(void);
static void render_emu_mix(void);
static void render_emu_bilinear(void);
static void render_quad(int viewportWidth, int viewportHeight);
static void update_system_texture(void);
static void update_debug_textures(void);
static void render_matrix(void);

bool renderer_init(void)
{
#if !defined(__APPLE__)
    int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);

    if (version == 0)
    {
        Log("GLAD ERROR: Failed to initialize OpenGL context");
        return false;
    }

    Log("GLAD: OpenGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
#endif

    renderer_opengl_version = (const char*)glGetString(GL_VERSION);
    Log("Using OpenGL %s", renderer_opengl_version);

    init_ogl_gui();
    init_ogl_emu();
    init_ogl_debug();

    first_frame = true;
    return true;
}

void renderer_destroy(void)
{
    glDeleteFramebuffers(1, &frame_buffer_object); 
    glDeleteTextures(1, &renderer_emu_texture);
    glDeleteTextures(1, &gameboy_texture);
    glDeleteTextures(1, &matrix_texture);
    glDeleteTextures(1, &renderer_emu_debug_vram_background);
    glDeleteTextures(2, renderer_emu_debug_vram_tiles);
    glDeleteTextures(40, renderer_emu_debug_vram_oam);
    ImGui_ImplOpenGL2_Shutdown();
}

void renderer_begin_render(void)
{
    ImGui_ImplOpenGL2_NewFrame();
}

void renderer_render(void)
{
    if (config_debug.debug)
    {
        update_debug_textures();
    }

    if (config_video.mix_frames)
        render_emu_mix();
    else
        render_emu_normal();

    if (config_video.matrix)
        render_matrix();

    render_emu_bilinear();

    ImVec4 clear_color = ImVec4(config_video.background_color[0], config_video.background_color[1], config_video.background_color[2], 1.00f);

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_gui();
}

void renderer_end_render(void)
{
#if defined(__APPLE__) || defined(_WIN32)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
#endif
}

static void init_ogl_gui(void)
{
    ImGui_ImplOpenGL2_Init();
}

static void init_ogl_emu(void)
{
    glEnable(GL_TEXTURE_2D);

    glGenFramebuffers(1, &frame_buffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glGenTextures(1, &renderer_emu_texture);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH * FRAME_BUFFER_SCALE, GAMEBOY_HEIGHT * FRAME_BUFFER_SCALE, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_emu_texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &gameboy_texture);  
    glBindTexture(GL_TEXTURE_2D, gameboy_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    init_matrix_textures();
}

static void init_ogl_debug(void)
{
    glGenTextures(1, &renderer_emu_debug_vram_background);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_background);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_background_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    for (int b = 0; b < 2; b++)
    {
        glGenTextures(1, &renderer_emu_debug_vram_tiles[b]);
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_tiles[b]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16 * 8, 24 * 8, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_tile_buffers[b]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    for (int s = 0; s < 40; s++)
    {
        glGenTextures(1, &renderer_emu_debug_vram_oam[s]);
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_oam[s]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_oam_buffers[s]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void init_matrix_textures(void)
{
    glGenTextures(1, &matrix_texture);

    glBindTexture(GL_TEXTURE_2D, matrix_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*) matrix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

static void render_gui(void)
{
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void render_emu_normal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glDisable(GL_BLEND);

    update_system_texture();

    render_quad(GAMEBOY_WIDTH * FRAME_BUFFER_SCALE, GAMEBOY_HEIGHT * FRAME_BUFFER_SCALE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_mix(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    float alpha = 0.15f + (0.50f * (1.0f - config_video.mix_frames_intensity));

    if (first_frame)
    {
        first_frame = false;
        alpha = 1.0f;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static bool round_error = false; 
    float round_color = 1.0f - (round_error ? 0.03f : 0.0f);
    round_error = !round_error;

    glEnable(GL_BLEND);
    glColor4f(round_color, round_color, round_color, alpha);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    update_system_texture();

    render_quad(GAMEBOY_WIDTH * FRAME_BUFFER_SCALE, GAMEBOY_HEIGHT * FRAME_BUFFER_SCALE);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void update_system_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, gameboy_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (config_video.bilinear)
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

static void update_debug_textures(void)
{
    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_background);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_background_buffer);

    for (int b = 0; b < 2; b++)
    {
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_tiles[b]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16 * 8, 24 * 8,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_tile_buffers[b]);
    }

    for (int s = 0; s < 40; s++)
    {
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_oam[s]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 16,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_oam_buffers[s]);
    }
}

static void render_emu_bilinear(void)
{
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (config_video.matrix)
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

static void render_quad(int viewportWidth, int viewportHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(viewportWidth, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(viewportWidth, viewportHeight);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0, viewportHeight);
    glEnd();
}

static void render_matrix(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
    glEnable(GL_BLEND);

    glColor4f(1.0f, 1.0f, 1.0f, config_video.matrix_intensity / 4.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, matrix_texture);

    int viewportWidth = GAMEBOY_WIDTH * FRAME_BUFFER_SCALE;
    int viewportHeight = GAMEBOY_HEIGHT * FRAME_BUFFER_SCALE;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(GAMEBOY_WIDTH, 0.0);
    glVertex2d(viewportWidth, 0.0);
    glTexCoord2d(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
    glVertex2d(viewportWidth, viewportHeight);
    glTexCoord2d(0.0, GAMEBOY_HEIGHT);
    glVertex2d(0.0, viewportHeight);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
