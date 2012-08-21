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

#include "RenderThread.h"
#include "GLFrame.h"

RenderThread::RenderThread(GLFrame*_GLFrame) : QThread(), m_pGLFrame(_GLFrame)
{
    m_bDoRendering = true;
    m_bDoResize = false;
    m_iFrameCounter = 0;
    m_iWidth = 0;
    m_iHeight = 0;
}

void RenderThread::resizeViewport(const QSize &size)
{
    m_iWidth = size.width();
    m_iHeight = size.height();
    m_bDoResize = true;
}

void RenderThread::stop()
{
    m_bDoRendering = false;
}

void RenderThread::run()
{
    m_pGLFrame->makeCurrent();
    GLInit();

    while (m_bDoRendering)
    {
        if (m_bDoResize)
        {
            GLResize(m_iWidth, m_iHeight);
            m_bDoResize = false;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        paintGL(); // render actual frame

        m_iFrameCounter++;
        m_pGLFrame->swapBuffers();

        //msleep(16); // wait 16ms => about 60 FPS
    }
}

void RenderThread::GLInit(void)
{
    glClearColor(0.05f, 0.05f, 0.1f, 0.0f); // Background => dark blue
}

void RenderThread::GLResize(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(45., ((GLfloat) width) / ((GLfloat) height), 0.1f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderThread::paintGL(void)
{
    glTranslatef(0.0f, 0.0f, -5.0f); // move 5 units into the screen
    glRotatef(m_iFrameCounter, 0.0f, 0.0f, 1.0f); // rotate z-axis
    glBegin(GL_QUADS);
    glColor3f(1., 1., 0.);
    glVertex3f(-1.0, -1.0, 0.0);
    glColor3f(1., 1., 1.);
    glVertex3f(1.0, -1.0, 0.0);
    glColor3f(1., 0., 1.);
    glVertex3f(1.0, 1.0, 0.0);
    glColor3f(1., 0., 0.);
    glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
}
