/* 
 * Gearboy Gameboy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * The full license is available at http://www.gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "gearboy.h"

#define SCREEN_WIDTH GAMEBOY_WIDTH
#define SCREEN_HEIGHT GAMEBOY_HEIGHT

const int modifier = 4;

int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
GB_Color* frameBuffer;
GearboyCore* gb;
bool keys[256];

void setupTexture()
{
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

    // Enable textures
    glEnable(GL_TEXTURE_2D);
}

void updateTexture()
{
    // Update pixels
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

    // Update Texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) screenData);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(display_width, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(display_width, display_height);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0, display_height);
    glEnd();
}

void display()
{
    gb->RunToVBlank(frameBuffer);

    glClear(GL_COLOR_BUFFER_BIT);
    updateTexture();
    glutSwapBuffers();
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

void reshape_window(GLsizei w, GLsizei h)
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // Resize quad
    display_width = w;
    display_height = h;
}

int main(int argc, char** argv)
{
    gb = new GearboyCore();
    gb->Init();
    //gb->LoadROM("/Users/nacho/Desktop/roms/Super Mario Land (JUE) (V1.0) [!].gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Super Mario Land 2 - 6 Golden Coins (UE) (V1.0) [!].gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Legend of Zelda, The - Link's Awakening (U) (V1.2) [!].gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Kirby's Dream Land (UE) [!].gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Snoopy - Magic Show (U) [!].gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Donkey Kong Land (U) [S][!].sgb");
    //Elevator Action (U) [!].gb
    //gb->LoadROM("/Users/nacho/Desktop/roms/Pokemon - Gold Version (UE) [C][!].gbc");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Pokemon - Edicion Azul (S) [S].sgb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Prehistorik Man (U).gb");
    //gb->LoadROM("/Users/nacho/Desktop/roms/Wave Race (UE) [!].gb");
    //if (gb->LoadROM("/Users/nacho/Desktop/roms/Legend of Zelda, The - Link's Awakening DX (U) (V1.0) [C][!].gbc"))
        if (gb->LoadROM("/Users/nacho/Desktop/roms/Pokemon - Gold Version (UE) [C][!].gbc"))
    {
        for (int i = 0; i < 256; i++)
            keys[i] = false;

        frameBuffer = new GB_Color[SCREEN_WIDTH * SCREEN_HEIGHT];

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

        glutInitWindowSize(display_width, display_height);
        glutInitWindowPosition(320, 320);
        glutCreateWindow("Gearboy");

        glutDisplayFunc(display);
        glutIdleFunc(display);
        glutReshapeFunc(reshape_window);
        glutKeyboardFunc(keyboard);
        glutKeyboardUpFunc(keyboardUP);
        glutIgnoreKeyRepeat(1);

        setupTexture();

        glutMainLoop();
    }

    SafeDeleteArray(frameBuffer);
    SafeDelete(gb);

    return 0;
}


