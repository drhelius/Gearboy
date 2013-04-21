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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <SDL/SDL.h>
#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "gearboy.h"

bool keys[256];
bool running = true;
EGLDisplay display;
EGLSurface surface;
EGLContext context;

const float kGB_Width = 160.0f;
const float kGB_Height = 144.0f;
const float kGB_TexWidth = kGB_Width / 256.0f;
const float kGB_TexHeight = kGB_Height / 256.0f;
const GLfloat kQuadTex[8] = { 0, 0, kGB_TexWidth, 0, kGB_TexWidth, kGB_TexHeight, 0, kGB_TexHeight};
GLshort quadVerts[8];

GearboyCore* theGearboyCore;
GB_Color* theFrameBuffer;
GLuint theGBTexture;

uint32_t screen_width, screen_height;

void update(void)
{
    SDL_Event keyevent;

    while (SDL_PollEvent(&keyevent))
    {
        switch(keyevent.type)
        {
            case SDL_KEYDOWN:
            switch(keyevent.key.keysym.sym)
            {
                case SDLK_LEFT:
                theGearboyCore->KeyPressed(Left_Key);
                break;
                case SDLK_RIGHT:
                theGearboyCore->KeyPressed(Right_Key);
                break;
                case SDLK_UP:
                theGearboyCore->KeyPressed(Up_Key);
                break;
                case SDLK_DOWN:
                theGearboyCore->KeyPressed(Down_Key);
                break;
                case SDLK_a:
                theGearboyCore->KeyPressed(B_Key);
                break;
                case SDLK_s:
                theGearboyCore->KeyPressed(A_Key);
                break;
                case SDLK_SPACE:
                theGearboyCore->KeyPressed(Select_Key);
                break;
                case SDLK_RETURN:
                theGearboyCore->KeyPressed(Start_Key);
                break;
                case SDLK_ESCAPE:
                running = false;
                break;
                default:
                break;
            }
            break;
            case SDL_KEYUP:
            switch(keyevent.key.keysym.sym)
            {
                case SDLK_LEFT:
                theGearboyCore->KeyReleased(Left_Key);
                break;
                case SDLK_RIGHT:
                theGearboyCore->KeyReleased(Right_Key);
                break;
                case SDLK_UP:
                theGearboyCore->KeyReleased(Up_Key);
                break;
                case SDLK_DOWN:
                theGearboyCore->KeyReleased(Down_Key);
                break;
                case SDLK_a:
                theGearboyCore->KeyReleased(B_Key);
                break;
                case SDLK_s:
                theGearboyCore->KeyReleased(A_Key);
                break;
                case SDLK_SPACE:
                theGearboyCore->KeyReleased(Select_Key);
                break;
                case SDLK_RETURN:
                theGearboyCore->KeyReleased(Start_Key);
                break;
                default:
                break;
            }
        }
    }

    theGearboyCore->RunToVBlank(NULL); // this is to force 30 FPS
    theGearboyCore->RunToVBlank(theFrameBuffer);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theFrameBuffer);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    eglSwapBuffers(display, surface);
}

void init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Log("SDL Error Init: %s", SDL_GetError());
    }

    if (SDL_SetVideoMode(0, 0, 32, SDL_SWSURFACE) == NULL)
    {
        Log("SDL Error Video: %s", SDL_GetError());
    }
}

void init_ogl(void)
{
    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_DISPMANX_ALPHA_T alpha;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLConfig config;

    // Get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display!=EGL_NO_DISPLAY);

    // Initialize the EGL display connection
    result = eglInitialize(display, NULL, NULL);
    assert(EGL_FALSE != result);

    // Get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);

    // Create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    assert(context!=EGL_NO_CONTEXT);

    // Create an EGL window surface
    success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
    assert( success >= 0 );

    int32_t zoom = screen_width / GAMEBOY_WIDTH;
    int32_t zoom2 = screen_height / GAMEBOY_HEIGHT;

    if (zoom2 < zoom)
        zoom = zoom2;

    int32_t display_width = GAMEBOY_WIDTH * zoom;
    int32_t display_height = GAMEBOY_HEIGHT * zoom;
    int32_t display_offset_x = (screen_width / 2) - (display_width / 2);
    int32_t display_offset_y = (screen_height / 2) - (display_height / 2);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = screen_width;
    dst_rect.height = screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = screen_width << 16;
    src_rect.height = screen_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
    alpha.opacity = 255;
    alpha.mask = 0;

    dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
        0/*layer*/, &dst_rect, 0/*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = screen_width;
    nativewindow.height = screen_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    surface = eglCreateWindowSurface( display, config, &nativewindow, NULL );
    assert(surface != EGL_NO_SURFACE);

    // Connect the context to the surface
    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);

    eglSwapInterval(display, 0);

    glGenTextures(1, &theGBTexture);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, theGBTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0.0f, 0.0f, screen_width, screen_height);

    quadVerts[0] = display_offset_x;
    quadVerts[1] = display_offset_y;
    quadVerts[2] = display_offset_x + display_width;
    quadVerts[3] = display_offset_y;
    quadVerts[4] = display_offset_x + display_width;
    quadVerts[5] = display_offset_y + display_height;
    quadVerts[6] = display_offset_x;
    quadVerts[7] = display_offset_y + display_height;

    glVertexPointer(2, GL_SHORT, 0, quadVerts);
    glEnableClientState(GL_VERTEX_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, 0, kQuadTex);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glClear(GL_COLOR_BUFFER_BIT);
}

void init(void)
{
    theGearboyCore = new GearboyCore();
    theGearboyCore->Init();

    theFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];

    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            int pixel = (y * GAMEBOY_WIDTH) + x;
            theFrameBuffer[pixel].red = theFrameBuffer[pixel].green = theFrameBuffer[pixel].blue = 0x00;
            theFrameBuffer[pixel].alpha = 0xFF;
        }
    }

    for (int i = 0; i < 256; i++)
    	keys[i] = false;

    bcm_host_init();
    init_sdl();
    init_ogl();
}

void end(void)
{
    SDL_Quit();
    bcm_host_deinit();
}

int main(int argc, char** argv)
{
    init();

    if (theGearboyCore->LoadROM(argv[1], false))
    {
        while (running)
        {
            update();
        }
    }

    SafeDeleteArray(theFrameBuffer);
    SafeDelete(theGearboyCore);

    end();

    return 0;
}
