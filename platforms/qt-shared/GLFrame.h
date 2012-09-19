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

#ifndef MYGLFRAME_H
#define MYGLFRAME_H

#include <QGLWidget>
#include "../../src/gearboy.h"
#include "RenderThread.h"

class Emulator;

class GLFrame : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLFrame(QWidget *parent = 0);
    ~GLFrame();
    void InitRenderThread(Emulator* pEmulator);
    void StopRenderThread();
    void PauseRenderThread();
    void ResumeRenderThread();
    bool IsRunningRenderThread();
    void SetBilinearFiletering(bool enabled);
    void SetMixFrames(bool enabled);

protected:
    void closeEvent(QCloseEvent *evt);
    void resizeEvent(QResizeEvent *evt);
    void paintEvent(QPaintEvent *);

private:
    RenderThread m_RenderThread;
};

#endif // MYGLFRAME_H
