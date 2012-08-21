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

#ifndef MYGLFRAME_H
#define MYGLFRAME_H

#include <QGLWidget>
#include "../../src/gearboy.h"
#include "RenderThread.h"

class GLFrame : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLFrame(QWidget *parent = 0);
    ~GLFrame();
    void initRenderThread(void);
    void stopRenderThread(void);

signals:

public slots:

protected:
    void closeEvent(QCloseEvent *evt);
    void resizeEvent(QResizeEvent *evt);
    void paintEvent(QPaintEvent *);

    RenderThread m_RenderThread;

private:
};

#endif // MYGLFRAME_H
