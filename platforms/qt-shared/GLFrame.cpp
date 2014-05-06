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

#include "GLFrame.h"
#include <QResizeEvent>
#include <QGLContext>

GLFrame::GLFrame(QWidget *parent) : QGLWidget(parent), m_RenderThread(this)
{
    setAutoBufferSwap(false);
}

GLFrame::~GLFrame()
{
}

void GLFrame::InitRenderThread(Emulator* pEmulator)
{
    doneCurrent();
    context()->moveToThread(&m_RenderThread);
    m_RenderThread.SetEmulator(pEmulator);
    m_RenderThread.start();
}

void GLFrame::StopRenderThread()
{
    m_RenderThread.Stop();
    m_RenderThread.wait();
}

void GLFrame::PauseRenderThread()
{
    m_RenderThread.Pause();
}

void GLFrame::ResumeRenderThread()
{
    m_RenderThread.Resume();
}

bool GLFrame::IsRunningRenderThread()
{
    return m_RenderThread.IsRunningEmulator();
}

void GLFrame::resizeEvent(QResizeEvent *evt)
{
    m_RenderThread.ResizeViewport(evt->size());
}

void GLFrame::paintEvent(QPaintEvent *)
{
    // Do nothing. Let the thread do the work
}

void GLFrame::closeEvent(QCloseEvent *evt)
{
    StopRenderThread();
    QGLWidget::closeEvent(evt);
}

void GLFrame::SetBilinearFiletering(bool enabled)
{
    m_RenderThread.SetBilinearFiletering(enabled);
}

void GLFrame::SetMixFrames(bool enabled)
{
    m_RenderThread.SetMixFrames(enabled);
}
