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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GLFrame.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_pUI = new Ui::MainWindow();
    m_pUI->setupUi(this);
    m_pGLFrame = new GLFrame();      // create our subclassed GLWidget
    setCentralWidget(m_pGLFrame);      // assign it to the central widget of the window
    m_pGLFrame->initRenderThread();    // start rendering
}

MainWindow::~MainWindow()
{
    SafeDelete(m_pGLFrame);
    SafeDelete(m_pUI);
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    m_pGLFrame->stopRenderThread();    // stop the thread befor exiting
    QMainWindow::closeEvent(evt);
}
