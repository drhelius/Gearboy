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
#include <QFileDialog>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GLFrame.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_pUI = new Ui::MainWindow();
    m_pUI->setupUi(this);
    
    m_pEmulator = new Emulator();
    m_pEmulator->Init();
    
    m_pGLFrame = new GLFrame();      // create our subclassed GLWidget
    setCentralWidget(m_pGLFrame);      // assign it to the central widget of the window
    m_pGLFrame->initRenderThread(m_pEmulator);    // start rendering
}

MainWindow::~MainWindow()
{
    SafeDelete(m_pEmulator);
    SafeDelete(m_pGLFrame);
    SafeDelete(m_pUI);
}

void MainWindow::MenuLoadROM()
{
    QString filename = QFileDialog::getOpenFileName( 
        this, 
        tr("Load ROM"), 
        QDir::currentPath(), 
        tr("Game Boy ROM files (*.gb *.gbc *.sgb);;All files (*.*)") );
    if( !filename.isNull() )
    {
        m_pEmulator->LoadRom(filename.toUtf8().data());
    }
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    m_pGLFrame->stopRenderThread();    // stop the thread befor exiting
    QMainWindow::closeEvent(evt);
}
