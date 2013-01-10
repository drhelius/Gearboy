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
#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "eglext.h"
#include "gearboy.h"

bool keys[256];
bool terminate = false;
EGLDisplay display;
EGLSurface surface;
EGLContext context;

const float kGB_Width = 160.0f;
const float kGB_Height = 144.0f;
const float kGB_TexWidth = kGB_Width / 256.0f;
const float kGB_TexHeight = kGB_Height / 256.0f;
const GLfloat box[] = {0.0f, kGB_Height, 1.0f, kGB_Width,kGB_Height, 1.0f, 0.0f, 0.0f, 1.0f, kGB_Width, 0.0f, 1.0f};
const GLfloat tex[] = {0.0f, 0.0f, kGB_TexWidth, 0.0f, 0.0f, kGB_TexHeight, kGB_TexWidth, kGB_TexHeight};

GearboyCore* theGearboyCore;
GB_Color* theFrameBuffer;
GB_Color* theTexture;
GLuint theGBTexture;

void draw(void)
{
    // Run emulator until next VBlank
    theGearboyCore->RunToVBlank(frameBuffer);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, GBTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, kGB_Width, 0.0f, kGB_Height, -100.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, GAMEBOY_WIDTH * 4, GAMEBOY_HEIGHT * 4);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    eglSwapBuffers(display, surface);
}

void update(void)
{
    theGearboyCore->RunToVBlank(theFrameBuffer);
    
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            theTexture[(y * 256) + x] = theFrameBuffer[(y * GAMEBOY_WIDTH) + x];
        }
    }
}

static void keyboard(unsigned char key, int x, int y)
{
    // Escape key
    if (key == 27)
    {
        gb->GetMemory()->MemoryDump("output.txt");
        exit(0);
    }
    else if (key == 'J' || key == 'j')
        gb->KeyPressed(A_Key);
    else if (key == 'K' || key == 'k')
        gb->KeyPressed(B_Key);
    else if (key == 'M' || key == 'm')
        gb->KeyPressed(Start_Key);
    else if (key == 'n' || key == 'n')
        gb->KeyPressed(Select_Key);
    else if (key == 'A' || key == 'a')
        gb->KeyPressed(Left_Key);
    else if (key == 'S' || key == 's')
        gb->KeyPressed(Down_Key);
    else if (key == 'D' || key == 'd')
        gb->KeyPressed(Right_Key);
    else if (key == 'W' || key == 'w')
        gb->KeyPressed(Up_Key);

    keys[key] = true;
}

static void keyboardUP(unsigned char key, int x, int y)
{
    if (key == 'J' || key == 'j')
        gb->KeyReleased(A_Key);
    else if (key == 'K' || key == 'k')
        gb->KeyReleased(B_Key);
    else if (key == 'M' || key == 'm')
        gb->KeyReleased(Start_Key);
    else if (key == 'n' || key == 'n')
        gb->KeyReleased(Select_Key);
    else if (key == 'A' || key == 'a')
        gb->KeyReleased(Left_Key);
    else if (key == 'S' || key == 's')
        gb->KeyReleased(Down_Key);
    else if (key == 'D' || key == 'd')
        gb->KeyReleased(Right_Key);
    else if (key == 'W' || key == 'w')
        gb->KeyReleased(Up_Key);

    keys[key] = false;
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

   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

   nativewindow.element = dispman_element;
   nativewindow.width = screen_width;
   nativewindow.height = screen_height;
   vc_dispmanx_update_submit_sync( dispman_update );

   surface = eglCreateWindowSurface( display, config, &nativewindow, NULL );
   assert(surface != EGL_NO_SURFACE);

   // Connect the context to the surface
   result = eglMakeCurrent(display, surface, surface, context);
   assert(EGL_FALSE != result);

    glGenTextures(1, &theGBTexture);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, box);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);
    
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, theGBTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void init(void)
{
    theGearboyCore = new GearboyCore();
    theGearboyCore->Init();
       
    theFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    theTexture = new GB_Color[256 * 256];
    
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            int pixel = (y * GAMEBOY_WIDTH) + x;
            theFrameBuffer[pixel].red = theFrameBuffer[pixel].green = theFrameBuffer[pixel].blue = 0x00;
            theFrameBuffer[pixel].alpha = 0xFF;
        }
    }
    
    for (int y = 0; y < 256; ++y)
    {
        for (int x = 0; x < 256; ++x)
        {
            int pixel = (y * 256) + x;
            theTexture[pixel].red = theTexture[pixel].green = theTexture[pixel].blue = 0x00;
            theTexture[pixel].alpha = 0xFF;
        }
    }
    
    for (int i = 0; i < 256; i++)
            keys[i] = false;
    
    init_ogl();
}


int main(int argc, char** argv)
{
    bcm_host_init();
    
    init();

    if (theGearboyCore->LoadROM("/home/pi/roms/testrom.gb", false))
    {
        while (!terminate)
        {
          update();
          draw();
        }
    }

    SafeDeleteArray(frameBuffer);
    SafeDelete(gb);

    return 0;
}
