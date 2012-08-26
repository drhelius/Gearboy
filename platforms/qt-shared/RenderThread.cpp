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
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "RenderThread.h"
#include "GLFrame.h"
#include "Emulator.h"

RenderThread::RenderThread(GLFrame* pGLFrame) : QThread(), m_pGLFrame(pGLFrame)
{
    m_bPaused = false;
    m_bDoRendering = true;
    m_bDoResize = false;
    m_pFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    m_iWidth = 0;
    m_iHeight = 0;
}

RenderThread::~RenderThread()
{
    SafeDeleteArray(m_pFrameBuffer);
}

void RenderThread::ResizeViewport(const QSize &size)
{
    m_iWidth = size.width();
    m_iHeight = size.height();
    m_bDoResize = true;
}

void RenderThread::Stop()
{
    m_bDoRendering = false;
}

void RenderThread::Pause()
{
    m_bPaused = true;
}

void RenderThread::Resume()
{
    m_bPaused = false;
}

bool RenderThread::IsRunningEmulator()
{
    return m_bDoRendering;
}

void RenderThread::SetEmulator(Emulator* pEmulator)
{
    m_pEmulator = pEmulator;
}

void RenderThread::run()
{
    m_pGLFrame->makeCurrent();
    Init();

    while (m_bDoRendering)
    {
        if (!m_bPaused)
        {
            m_pEmulator->RunToVBlank(m_pFrameBuffer);

            if (m_bDoResize)
            {
                Resize(m_iWidth, m_iHeight);
                m_bDoResize = false;
            }

            RenderFrame();
            m_pGLFrame->swapBuffers();
        }

        //msleep(16); // wait 16ms => about 60 FPS
    }
}

void RenderThread::Init()
{
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            int pixel = (y * GAMEBOY_WIDTH) + x;
            m_pFrameBuffer[pixel].red = m_pFrameBuffer[pixel].green =
                    m_pFrameBuffer[pixel].blue = m_pFrameBuffer[pixel].alpha = 0;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glEnable(GL_TEXTURE_2D);
}

void RenderThread::Resize(int width, int height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, height, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, width, height);
}

void RenderThread::RenderFrame()
{
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(m_iWidth, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(m_iWidth, m_iHeight);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0, m_iHeight);
    glEnd();
}
