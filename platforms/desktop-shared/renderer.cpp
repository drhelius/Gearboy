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
#include <GL/glew.h>
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
static uint32_t frame_buffer_object;
static bool first_frame;

static void init_gui(void);
static void init_emu(void);
static void render_gui(void);
static void render_emu_normal(void);
static void render_emu_mix(void);
static void render_emu_bilinear(void);
static void render_quad(int viewportWidth, int viewportHeight);

void renderer_init(void)
{
    #ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        Log("GLEW Error: %s\n", glewGetErrorString(err));
    }
    Log("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    init_gui();
    init_emu();

    first_frame = true;
}

void renderer_destroy(void)
{
    glDeleteFramebuffers(1, &frame_buffer_object); 
    glDeleteTextures(1, &renderer_emu_texture);
    glDeleteTextures(1, &gameboy_texture);
    ImGui_ImplOpenGL2_Shutdown();
}

void renderer_begin_render(void)
{
    ImGui_ImplOpenGL2_NewFrame();
}

void renderer_render(void)
{
    if (config_video.mix_frames)
        render_emu_mix();
    else
        render_emu_normal();

    render_emu_bilinear();

    ImVec4 clear_color = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_gui();
}

void renderer_end_render(void)
{

}

static void init_gui(void)
{
    ImGui_ImplOpenGL2_Init();
}

static void init_emu(void)
{
    glEnable(GL_TEXTURE_2D);

    glGenFramebuffers(1, &frame_buffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glGenTextures(1, &renderer_emu_texture);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_emu_texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &gameboy_texture);  
    glBindTexture(GL_TEXTURE_2D, gameboy_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void render_gui(void)
{
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void render_emu_normal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, gameboy_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);

    render_quad(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_mix(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    float alpha = 0.25f;

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

    glBindTexture(GL_TEXTURE_2D, gameboy_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);

    render_quad(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_bilinear(void)
{
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);

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
