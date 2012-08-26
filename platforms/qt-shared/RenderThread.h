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

#ifndef MYRENDERTHREAD_H
#define MYRENDERTHREAD_H

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
    void ResizeViewport(const QSize &size);
    void run();
    void Stop();
    void Pause();
    void Resume();
    void SetEmulator(Emulator* pEmulator);
    bool IsRunningEmulator();

protected:
    void Init();
    void Resize(int width, int height);
    void RenderFrame();

private:
    bool m_bDoRendering, m_bDoResize, m_bPaused;
    int m_iWidth, m_iHeight;
    GLFrame *m_pGLFrame;
    Emulator* m_pEmulator;
    GB_Color* m_pFrameBuffer;

signals:
public slots:
};

#endif // MYRENDERTHREAD_H
