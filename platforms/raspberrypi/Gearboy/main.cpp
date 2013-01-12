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
#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "gearboy.h"

struct Tex_Color
{
    u8 red;
    u8 green;
    u8 blue;
};

bool keys[256];
bool terminate = false;
EGLDisplay display;
EGLSurface surface;
EGLContext context;

const float kGB_Width = 160.0f;
const float kGB_Height = 144.0f;
const float kGB_TexWidth = kGB_Width / 256.0f;
const float kGB_TexHeight = kGB_Height / 256.0f;
const GLfloat kQuadTex[8] = { 0, 0, kGB_TexWidth, 0, kGB_TexWidth, kGB_TexHeight, 0, kGB_TexHeight};
const GLshort kQuadVerts[8] = { 0.0f, 0.0f, GAMEBOY_WIDTH, 0.0f, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0.0f, GAMEBOY_HEIGHT};

GearboyCore* theGearboyCore;
GB_Color* theFrameBuffer;
Tex_Color* theTexture;
GLuint theGBTexture;

uint32_t screen_width, screen_height;

void draw(void)
{
    //glClear(GL_COLOR_BUFFER_BIT);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 144, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    eglSwapBuffers(display, surface);
}

void update(void)
{
    theGearboyCore->RunToVBlank(theFrameBuffer);
    
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        int y_offset_tex = y * 256;
        int y_offset_gb = y * GAMEBOY_WIDTH;
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
        	int tex_offset = y_offset_tex + x;
        	int buff_offset = y_offset_gb + x;
            theTexture[tex_offset].red = theFrameBuffer[buff_offset].red;
            theTexture[tex_offset].green = theFrameBuffer[buff_offset].green;
            theTexture[tex_offset].blue = theFrameBuffer[buff_offset].blue;
        }
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

    screen_width = 256;
screen_height = 256;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, screen_width, screen_height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0.0f, 0.0f, screen_width, screen_height);

    glVertexPointer(2, GL_SHORT, 0, kQuadVerts);
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
    theTexture = new Tex_Color[256 * 256];
    
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
    
    int frames = 0;
    timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);

    if (theGearboyCore->LoadROM("/home/pi/roms/testrom.gb", false))
    {
        while (!terminate)
        {
          update();
          //draw();
          frames++;
          gettimeofday(&t2, NULL);
          elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    	  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    	  if (elapsedTime > 1.0)
    	  {
    	  	Log("FPS %f", frames / elpasedTime);
    	  	frames = 0;
    	  	t1=t2;
    	  	}
          
        }
    }

    SafeDeleteArray(theFrameBuffer);
    SafeDelete(theGearboyCore);

    return 0;
}
