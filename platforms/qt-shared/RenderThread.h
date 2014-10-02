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

#ifndef MYRENDERTHREAD_H
#define MYRENDERTHREAD_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#endif
#include <QThread>
#include "../../src/gearboy.h"

class Emulator;
class GLFrame;
class QSize;

class RenderThread : public QThread
{
    Q_OBJECT

public:
    explicit RenderThread(GLFrame *pGLFrame = 0);
    virtual ~RenderThread();
    void ResizeViewport(const QSize &size, int pixel_ratio);
    void run();
    void Stop();
    void Pause();
    void Resume();
    void SetEmulator(Emulator* pEmulator);
    bool IsRunningEmulator();
    void SetBilinearFiletering(bool enabled);
    void SetMixFrames(bool enabled);

protected:
    void Init();
    void RenderFrame();
    void RenderMixFrames();
    void RenderQuad(int viewportWidth, int viewportHeight, bool mirrorY);
    void SetupTexture(GLvoid* data);

private:
    bool m_bDoRendering, m_bPaused;
    int m_iWidth, m_iHeight;
    GLFrame *m_pGLFrame;
    Emulator* m_pEmulator;
    GB_Color* m_pFrameBuffer;
    bool m_bFiltering;
    bool m_bMixFrames;
    bool m_bResizeEvent;
    GLuint m_AccumulationFramebuffer;
    GLuint m_AccumulationTexture;
    GLuint m_GBTexture;
    bool m_bFirstFrame;
};

#endif // MYRENDERTHREAD_H
