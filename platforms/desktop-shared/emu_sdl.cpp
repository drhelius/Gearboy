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
#include <SDL_opengl.h>
#include "../../src/gearboy.h"
#include "emu_imgui.h"

#define EMU_SDL_IMPORT
#include "emu_sdl.h"

int emu_sdl_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        Log("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    emu_sdl_window = SDL_CreateWindow(GEARBOY_TITLE " " GEARBOY_VERSION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 700, window_flags);
    emu_sdl_gl_context = SDL_GL_CreateContext(emu_sdl_window);
    SDL_GL_MakeCurrent(emu_sdl_window, emu_sdl_gl_context);
    SDL_GL_SetSwapInterval(0);

    SDL_SetWindowMinimumSize(emu_sdl_window, 680, 630);

    emu_imgui_init();

    return 0;
}

void emu_sdl_destroy(void)
{
    emu_imgui_destroy();

    SDL_GL_DeleteContext(emu_sdl_gl_context);
    SDL_DestroyWindow(emu_sdl_window);
    SDL_Quit();
}

void emu_sdl_mainloop(void)
{
    bool done = false;
    
    while (!done)
    {
        SDL_Event event;
        
        while (SDL_PollEvent(&event))
        {
            emu_imgui_event(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }       

        emu_imgui_update();

        SDL_GL_SwapWindow(emu_sdl_window);
    }    
}