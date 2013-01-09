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
#include "EGL/eglext.h"


#include "../../../src/gearboy.h"

#define SCREEN_WIDTH GAMEBOY_WIDTH
#define SCREEN_HEIGHT GAMEBOY_HEIGHT

const int modifier = 4;

int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;
uint32_t screen_width;
uint32_t screen_height;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
GB_Color* frameBuffer;
GearboyCore* gb;
bool keys[256];
bool terminate = false;
EGLDisplay display;
EGLSurface surface;
EGLContext context;

void setupTexture()
{
    // Create the framebuffer for the emulator
    frameBuffer = new GB_Color[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    // Clear screen
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
        for (int x = 0; x < SCREEN_WIDTH; ++x)
            screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

    // Create a texture 
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) screenData);

    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Enable the texture
    glEnable(GL_TEXTURE_2D);
}

void updateTexture()
{
    // Update texture data from emulator buffer
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            int pixel = (y * SCREEN_WIDTH) + x;
            screenData[y][x][0] = frameBuffer[pixel].red;
            screenData[y][x][1] = frameBuffer[pixel].green;
            screenData[y][x][2] = frameBuffer[pixel].blue;
        }
    }

    // Upload data to texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) screenData);

    // Draw screen quad
    draw(0, 0, display_width, display_height);
}

void display()
{
    // Run emulator until next VBlank
    gb->RunToVBlank(frameBuffer);

    // Draw the emulator quad
    glClear(GL_COLOR_BUFFER_BIT);
    updateTexture();
    eglSwapBuffers(display, surface);
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

void draw( short x, short y, short w, short h )
{
    const GLshort t[8] = { 0, 0, 1, 0, 1, 1, 0, 1 };
    const GLshort v[8] = { x, y, x+w, y, x+w, y+h, x, y+h };

    glVertexPointer( 2, GL_SHORT, 0, v );
    glEnableClientState( GL_VERTEX_ARRAY );

    glTexCoordPointer( 2, GL_SHORT, 0, t );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
}

void init_ogl(void)
{
   int32_t success = 0;
   EGLBoolean result;
   EGLint num_config;

   EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;

   const EGLint attribute_list[] =
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
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = screen_width;
   nativewindow.height = screen_height;
   vc_dispmanx_update_submit_sync( dispman_update );
      
   surface = eglCreateWindowSurface( display, config, &nativewindow, NULL );
   assert(surface != EGL_NO_SURFACE);

   // Connect the context to the surface
   result = eglMakeCurrent(display, surface, surface, context);
   assert(EGL_FALSE != result);


   // Setup 2D view
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D(0, w, h, 0);
    glOrtho(0,  w,  h,  0,  -1,  1)
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}


int main(int argc, char** argv)
{
    bcm_host_init();
    
    init_ogl();

    setupTexture();

    gb = new GearboyCore();
    gb->Init();
            
    if (gb->LoadROM("/home/pi/Desktop/roms/testrom.gb", false))
    {
        for (int i = 0; i < 256; i++)
            keys[i] = false;

        while (!terminate)
        {
          display();
        }
    }

    SafeDeleteArray(frameBuffer);
    SafeDelete(gb);

    return 0;
}
