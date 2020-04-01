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

static void init_gui(void);
static void init_emu(void);
static void render_gui(void);
static void render_emu(void);

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
}

void renderer_destroy(void)
{
    glDeleteTextures(1, &renderer_emu_texture);
    ImGui_ImplOpenGL2_Shutdown();
}

void renderer_begin_render(void)
{
    ImGui_ImplOpenGL2_NewFrame();
}

void renderer_render(void)
{
    ImVec4 clear_color = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_emu();
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
    glGenTextures(1, &renderer_emu_texture);  
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);
}

static void render_gui(void)
{
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void render_emu(void)
{
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid*) emu_frame_buffer);

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
