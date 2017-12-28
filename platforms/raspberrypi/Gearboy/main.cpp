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
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <libconfig.h++>
#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "gearboy.h"

using namespace std;
using namespace libconfig;

bool running = true;
bool paused = false;

EGLDisplay display;
EGLSurface surface;
EGLContext context;

static const char *output_file = "gearboy.cfg";

const float kGB_Width = 160.0f;
const float kGB_Height = 144.0f;
const float kGB_TexWidth = kGB_Width / 256.0f;
const float kGB_TexHeight = kGB_Height / 256.0f;
const GLfloat kQuadTex[8] = { 0, 0, kGB_TexWidth, 0, kGB_TexWidth, kGB_TexHeight, 0, kGB_TexHeight};
GLshort quadVerts[8];

GearboyCore* theGearboyCore;
Sound_Queue* theSoundQueue;
GB_Color* theFrameBuffer;
GLuint theGBTexture;
s16 theSampleBufffer[AUDIO_BUFFER_SIZE];

bool audioEnabled = true;

struct palette_color
{
    int red;
    int green;
    int blue;
    int alpha;
};

palette_color dmg_palette[4];

SDL_Joystick* game_pad = NULL;
SDL_Keycode kc_keypad_left, kc_keypad_right, kc_keypad_up, kc_keypad_down, kc_keypad_a, kc_keypad_b, kc_keypad_start, kc_keypad_select, kc_emulator_pause, kc_emulator_quit;
bool jg_x_axis_invert, jg_y_axis_invert;
int jg_a, jg_b, jg_start, jg_select, jg_x_axis, jg_y_axis;

uint32_t screen_width, screen_height;

SDL_Window* theWindow;

void update(void)
{
    SDL_Event keyevent;

    while (SDL_PollEvent(&keyevent))
    {
        switch(keyevent.type)
        {
            case SDL_QUIT:
            running = false;
            break;

            case SDL_JOYBUTTONDOWN:
            {
                if (keyevent.jbutton.button == jg_b)
                    theGearboyCore->KeyPressed(B_Key);
                else if (keyevent.jbutton.button == jg_a)
                    theGearboyCore->KeyPressed(A_Key);
                else if (keyevent.jbutton.button == jg_select)
                    theGearboyCore->KeyPressed(Select_Key);
                else if (keyevent.jbutton.button == jg_start)
                    theGearboyCore->KeyPressed(Start_Key);
            }
            break;

            case SDL_JOYBUTTONUP:
            {
                if (keyevent.jbutton.button == jg_b)
                    theGearboyCore->KeyReleased(B_Key);
                else if (keyevent.jbutton.button == jg_a)
                    theGearboyCore->KeyReleased(A_Key);
                else if (keyevent.jbutton.button == jg_select)
                    theGearboyCore->KeyReleased(Select_Key);
                else if (keyevent.jbutton.button == jg_start)
                    theGearboyCore->KeyReleased(Start_Key);
            }
            break;

            case SDL_JOYAXISMOTION:
            {
                if(keyevent.jaxis.axis == jg_x_axis)
                {
                    int x_motion = keyevent.jaxis.value * (jg_x_axis_invert ? -1 : 1);
                    if (x_motion < 0)
                        theGearboyCore->KeyPressed(Left_Key);
                    else if (x_motion > 0)
                        theGearboyCore->KeyPressed(Right_Key);
                    else
                    {
                        theGearboyCore->KeyReleased(Left_Key);
                        theGearboyCore->KeyReleased(Right_Key);
                    }
                }
                else if(keyevent.jaxis.axis == jg_y_axis)
                {
                    int y_motion = keyevent.jaxis.value * (jg_y_axis_invert ? -1 : 1);
                    if (y_motion < 0)
                        theGearboyCore->KeyPressed(Up_Key);
                    else if (y_motion > 0)
                        theGearboyCore->KeyPressed(Down_Key);
                    else
                    {
                        theGearboyCore->KeyReleased(Up_Key);
                        theGearboyCore->KeyReleased(Down_Key);
                    }
                }
            }
            break;

            case SDL_KEYDOWN:
            {
                if (keyevent.key.keysym.sym == kc_keypad_left)
                    theGearboyCore->KeyPressed(Left_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_right)
                    theGearboyCore->KeyPressed(Right_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_up)
                    theGearboyCore->KeyPressed(Up_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_down)
                    theGearboyCore->KeyPressed(Down_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_b)
                    theGearboyCore->KeyPressed(B_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_a)
                    theGearboyCore->KeyPressed(A_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_select)
                    theGearboyCore->KeyPressed(Select_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_start)
                    theGearboyCore->KeyPressed(Start_Key);

                if (keyevent.key.keysym.sym == kc_emulator_quit)
                    running = false;
                else if (keyevent.key.keysym.sym == kc_emulator_pause)
                {
                    paused = !paused;
                    theGearboyCore->Pause(paused);
                }
            }
            break;

            case SDL_KEYUP:
            {
                if (keyevent.key.keysym.sym == kc_keypad_left)
                    theGearboyCore->KeyReleased(Left_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_right)
                    theGearboyCore->KeyReleased(Right_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_up)
                    theGearboyCore->KeyReleased(Up_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_down)
                    theGearboyCore->KeyReleased(Down_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_b)
                    theGearboyCore->KeyReleased(B_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_a)
                    theGearboyCore->KeyReleased(A_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_select)
                    theGearboyCore->KeyReleased(Select_Key);
                else if (keyevent.key.keysym.sym == kc_keypad_start)
                    theGearboyCore->KeyReleased(Start_Key);
            }
            break;
        }
    }

    int sampleCount = 0;

    theGearboyCore->RunToVBlank(theFrameBuffer, theSampleBufffer, &sampleCount);

    if (audioEnabled && (sampleCount > 0))
    {
        theSoundQueue->write(theSampleBufffer, sampleCount);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theFrameBuffer);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    eglSwapBuffers(display, surface);
}

void init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        Log("SDL Error Init: %s", SDL_GetError());
    }

    theWindow = SDL_CreateWindow("Gearboy", 0, 0, 0, 0, 0);

    if (theWindow == NULL)
    {
        Log("SDL Error Video: %s", SDL_GetError());
    }

    SDL_ShowCursor(SDL_DISABLE);

    game_pad = SDL_JoystickOpen(0);

    if(game_pad == NULL)
    {
        Log("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }

    kc_keypad_left = SDLK_LEFT;
    kc_keypad_right = SDLK_RIGHT;
    kc_keypad_up = SDLK_UP;
    kc_keypad_down = SDLK_DOWN;
    kc_keypad_a = SDLK_a;
    kc_keypad_b = SDLK_s;
    kc_keypad_start = SDLK_RETURN;
    kc_keypad_select = SDLK_SPACE;
    kc_emulator_pause = SDLK_p;
    kc_emulator_quit = SDLK_ESCAPE;

    jg_x_axis_invert = false;
    jg_y_axis_invert = false;
    jg_a = 1;
    jg_b = 2;
    jg_start = 9;
    jg_select = 8;
    jg_x_axis = 0;
    jg_y_axis = 1;

    dmg_palette[0].red = 0xEF;
    dmg_palette[0].green = 0xF3;
    dmg_palette[0].blue = 0xD5;
    dmg_palette[0].alpha = 0xFF;

    dmg_palette[1].red = 0xA3;
    dmg_palette[1].green = 0xB6;
    dmg_palette[1].blue = 0x7A;
    dmg_palette[1].alpha = 0xFF;

    dmg_palette[2].red = 0x37;
    dmg_palette[2].green = 0x61;
    dmg_palette[2].blue = 0x3B;
    dmg_palette[2].alpha = 0xFF;

    dmg_palette[3].red = 0x04;
    dmg_palette[3].green = 0x1C;
    dmg_palette[3].blue = 0x16;
    dmg_palette[3].alpha = 0xFF;

    Config cfg;

    try
    {
        cfg.readFile(output_file);

        try
        {
            const Setting& root = cfg.getRoot();
            const Setting &gearboy = root["Gearboy"];

            string keypad_left, keypad_right, keypad_up, keypad_down, keypad_a, keypad_b,
            keypad_start, keypad_select, emulator_pause, emulator_quit;
            gearboy.lookupValue("keypad_left", keypad_left);
            gearboy.lookupValue("keypad_right", keypad_right);
            gearboy.lookupValue("keypad_up", keypad_up);
            gearboy.lookupValue("keypad_down", keypad_down);
            gearboy.lookupValue("keypad_a", keypad_a);
            gearboy.lookupValue("keypad_b", keypad_b);
            gearboy.lookupValue("keypad_start", keypad_start);
            gearboy.lookupValue("keypad_select", keypad_select);

            gearboy.lookupValue("joystick_gamepad_a", jg_a);
            gearboy.lookupValue("joystick_gamepad_b", jg_b);
            gearboy.lookupValue("joystick_gamepad_start", jg_start);
            gearboy.lookupValue("joystick_gamepad_select", jg_select);
            gearboy.lookupValue("joystick_gamepad_x_axis", jg_x_axis);
            gearboy.lookupValue("joystick_gamepad_y_axis", jg_y_axis);
            gearboy.lookupValue("joystick_gamepad_x_axis_invert", jg_x_axis_invert);
            gearboy.lookupValue("joystick_gamepad_y_axis_invert", jg_y_axis_invert);

            gearboy.lookupValue("emulator_pause", emulator_pause);
            gearboy.lookupValue("emulator_quit", emulator_quit);

            gearboy.lookupValue("emulator_pal0_red", dmg_palette[0].red);
            gearboy.lookupValue("emulator_pal0_green", dmg_palette[0].green);
            gearboy.lookupValue("emulator_pal0_blue", dmg_palette[0].blue);
            gearboy.lookupValue("emulator_pal1_red", dmg_palette[1].red);
            gearboy.lookupValue("emulator_pal1_green", dmg_palette[1].green);
            gearboy.lookupValue("emulator_pal1_blue", dmg_palette[1].blue);
            gearboy.lookupValue("emulator_pal2_red", dmg_palette[2].red);
            gearboy.lookupValue("emulator_pal2_green", dmg_palette[2].green);
            gearboy.lookupValue("emulator_pal2_blue", dmg_palette[2].blue);
            gearboy.lookupValue("emulator_pal3_red", dmg_palette[3].red);
            gearboy.lookupValue("emulator_pal3_green", dmg_palette[3].green);
            gearboy.lookupValue("emulator_pal3_blue", dmg_palette[3].blue);

            kc_keypad_left = SDL_GetKeyFromName(keypad_left.c_str());
            kc_keypad_right = SDL_GetKeyFromName(keypad_right.c_str());
            kc_keypad_up = SDL_GetKeyFromName(keypad_up.c_str());
            kc_keypad_down = SDL_GetKeyFromName(keypad_down.c_str());
            kc_keypad_a = SDL_GetKeyFromName(keypad_a.c_str());
            kc_keypad_b = SDL_GetKeyFromName(keypad_b.c_str());
            kc_keypad_start = SDL_GetKeyFromName(keypad_start.c_str());
            kc_keypad_select = SDL_GetKeyFromName(keypad_select.c_str());

            kc_emulator_pause = SDL_GetKeyFromName(emulator_pause.c_str());
            kc_emulator_quit = SDL_GetKeyFromName(emulator_quit.c_str());
        }
        catch(const SettingNotFoundException &nfex)
        {
            std::cerr << "Setting not found" << std::endl;
        }
    }
    catch(const FileIOException &fioex)
    {
        Log("I/O error while reading file: %s", output_file);
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
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

    eglSwapInterval(display, 1);

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

    bcm_host_init();
    init_ogl();
    init_sdl();

    theGearboyCore = new GearboyCore();
    theGearboyCore->Init();

    theSoundQueue = new Sound_Queue();
    theSoundQueue->start(44100, 2);

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
}

void end(void)
{
    Config cfg;

    Setting &root = cfg.getRoot();
    Setting &address = root.add("Gearboy", Setting::TypeGroup);

    address.add("keypad_left", Setting::TypeString) = SDL_GetKeyName(kc_keypad_left);
    address.add("keypad_right", Setting::TypeString) = SDL_GetKeyName(kc_keypad_right);
    address.add("keypad_up", Setting::TypeString) = SDL_GetKeyName(kc_keypad_up);
    address.add("keypad_down", Setting::TypeString) = SDL_GetKeyName(kc_keypad_down);
    address.add("keypad_a", Setting::TypeString) = SDL_GetKeyName(kc_keypad_a);
    address.add("keypad_b", Setting::TypeString) = SDL_GetKeyName(kc_keypad_b);
    address.add("keypad_start", Setting::TypeString) = SDL_GetKeyName(kc_keypad_start);
    address.add("keypad_select", Setting::TypeString) = SDL_GetKeyName(kc_keypad_select);

    address.add("joystick_gamepad_a", Setting::TypeInt) = jg_a;
    address.add("joystick_gamepad_b", Setting::TypeInt) = jg_b;
    address.add("joystick_gamepad_start", Setting::TypeInt) = jg_start;
    address.add("joystick_gamepad_select", Setting::TypeInt) = jg_select;
    address.add("joystick_gamepad_x_axis", Setting::TypeInt) = jg_x_axis;
    address.add("joystick_gamepad_y_axis", Setting::TypeInt) = jg_y_axis;
    address.add("joystick_gamepad_x_axis_invert", Setting::TypeBoolean) = jg_x_axis_invert;
    address.add("joystick_gamepad_y_axis_invert", Setting::TypeBoolean) = jg_y_axis_invert;

    address.add("emulator_pause", Setting::TypeString) = SDL_GetKeyName(kc_emulator_pause);
    address.add("emulator_quit", Setting::TypeString) = SDL_GetKeyName(kc_emulator_quit);

    address.add("emulator_pal0_red", Setting::TypeInt) = dmg_palette[0].red;
    address.add("emulator_pal0_green", Setting::TypeInt) = dmg_palette[0].green;
    address.add("emulator_pal0_blue", Setting::TypeInt) = dmg_palette[0].blue;
    address.add("emulator_pal1_red", Setting::TypeInt) = dmg_palette[1].red;
    address.add("emulator_pal1_green", Setting::TypeInt) = dmg_palette[1].green;
    address.add("emulator_pal1_blue", Setting::TypeInt) = dmg_palette[1].blue;
    address.add("emulator_pal2_red", Setting::TypeInt) = dmg_palette[2].red;
    address.add("emulator_pal2_green", Setting::TypeInt) = dmg_palette[2].green;
    address.add("emulator_pal2_blue", Setting::TypeInt) = dmg_palette[2].blue;
    address.add("emulator_pal3_red", Setting::TypeInt) = dmg_palette[3].red;
    address.add("emulator_pal3_green", Setting::TypeInt) = dmg_palette[3].green;
    address.add("emulator_pal3_blue", Setting::TypeInt) = dmg_palette[3].blue;

    try
    {
        cfg.writeFile(output_file);
    }
    catch(const FileIOException &fioex)
    {
        Log("I/O error while writing file: %s", output_file);
    }

    SDL_JoystickClose(game_pad);

    SafeDeleteArray(theFrameBuffer);
    SafeDelete(theSoundQueue);
    SafeDelete(theGearboyCore);
    SDL_DestroyWindow(theWindow);
    SDL_Quit();
    bcm_host_deinit();
}

int main(int argc, char** argv)
{
    init();

    if (argc < 2 || argc > 4)
    {
        end();
        printf("usage: %s rom_path [options]\n", argv[0]);
        printf("options:\n-nosound\n-forcedmg\n");
        return -1;
    }

    bool forcedmg = false;

    if (argc > 2)
    {
        for (int i = 2; i < argc; i++)
        {
            if (strcmp("-nosound", argv[i]) == 0)
                audioEnabled = false;
            else if (strcmp("-forcedmg", argv[i]) == 0)
                forcedmg = true;
            else
            {
                end();
                printf("invalid option: %s\n", argv[i]);
                return -1;
            }
        }
    }

    if (theGearboyCore->LoadROM(argv[1], forcedmg))
    {
        GB_Color color1;
        GB_Color color2;
        GB_Color color3;
        GB_Color color4;

        color1.red = dmg_palette[0].red;
        color1.green = dmg_palette[0].green;
        color1.blue = dmg_palette[0].blue;
        color2.red = dmg_palette[1].red;
        color2.green = dmg_palette[1].green;
        color2.blue = dmg_palette[1].blue;
        color3.red = dmg_palette[2].red;
        color3.green = dmg_palette[2].green;
        color3.blue = dmg_palette[2].blue;
        color4.red = dmg_palette[3].red;
        color4.green = dmg_palette[3].green;
        color4.blue = dmg_palette[3].blue;

        theGearboyCore->SetDMGPalette(color1, color2, color3, color4);
        theGearboyCore->LoadRam();

        while (running)
        {
            update();
        }

        theGearboyCore->SaveRam();
    }

    end();

    return 0;
}
