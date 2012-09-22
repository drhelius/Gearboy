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

#include "RenderThread.h"
#include "GLFrame.h"
#include "Emulator.h"

const float kMixFrameAlpha = 0.55f;

RenderThread::RenderThread(GLFrame* pGLFrame) : QThread(), m_pGLFrame(pGLFrame)
{
    m_bPaused = false;
    m_bDoRendering = true;
    m_pFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    m_iWidth = 0;
    m_iHeight = 0;
    InitPointer(m_pEmulator);
    m_bFiltering = false;
    m_bMixFrames = true;
}

RenderThread::~RenderThread()
{
    m_pGLFrame->makeCurrent();
    SafeDeleteArray(m_pFrameBuffer);
}

void RenderThread::ResizeViewport(const QSize &size)
{
    m_iWidth = size.width();
    m_iHeight = size.height();
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
            if (m_bMixFrames)
                RenderMixFrames();
            else
                RenderFrame();
            m_pGLFrame->swapBuffers();
        }
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
                    m_pFrameBuffer[pixel].blue = 0x00;
            m_pFrameBuffer[pixel].alpha = 0xFF;
        }
    }

    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_ACCUM_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void RenderThread::RenderFrame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);
    if (m_bFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    RenderQuad(m_iWidth, m_iHeight);
}

void RenderThread::RenderMixFrames()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);

    if (m_bFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    RenderQuad(m_iWidth, m_iHeight);

    glAccum(GL_MULT, kMixFrameAlpha);
    glAccum(GL_ACCUM, 1.0f - kMixFrameAlpha);
    glAccum(GL_RETURN, 1.0f);
}

void RenderThread::RenderQuad(int viewportWidth, int viewportHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewportWidth, viewportHeight, 0);
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

void RenderThread::SetBilinearFiletering(bool enabled)
{
    m_bFiltering = enabled;
}

void RenderThread::SetMixFrames(bool enabled)
{
    m_bMixFrames = enabled;
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_ACCUM_BUFFER_BIT);
}

