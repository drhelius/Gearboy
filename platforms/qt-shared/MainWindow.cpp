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

    QObject::connect(m_pUI->menuGame_Boy, SIGNAL(aboutToShow()), this, SLOT(MenuPressed()));
    QObject::connect(m_pUI->menuGame_Boy, SIGNAL(aboutToHide()), this, SLOT(MenuReleased()));

    m_pEmulator = new Emulator();
    m_pEmulator->Init();

    QGLFormat f;
    f.setSwapInterval(1);
    QGLFormat::setDefaultFormat(f);

    m_pGLFrame = new GLFrame();

    setCentralWidget(m_pGLFrame);
    m_pGLFrame->InitRenderThread(m_pEmulator);
}

MainWindow::~MainWindow()
{
    SafeDelete(m_pEmulator);
    SafeDelete(m_pGLFrame);
    SafeDelete(m_pUI);
}

void MainWindow::MenuGameBoyLoadROM()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Load ROM"),
            QDir::currentPath(),
            tr("Game Boy ROM files (*.gb *.gbc *.sgb);;All files (*.*)"));
    if (!filename.isNull())
    {
        m_pEmulator->LoadRom(filename.toUtf8().data());
    }
}

void MainWindow::MenuGameBoyPause()
{
}

void MainWindow::MenuGameBoyReset()
{
}

void MainWindow::MenuGameBoySelectStateSlot()
{
}

void MainWindow::MenuGameBoySaveState()
{
}

void MainWindow::MenuGameBoyLoadState()
{
}

void MainWindow::MenuGameBoySaveStateAs()
{
}

void MainWindow::MenuGameBoyLoadStateFrom()
{
}

void MainWindow::MenuSettingsInput()
{
}

void MainWindow::MenuSettingsVideo()
{
}

void MainWindow::MenuSettingsSound()
{
}

void MainWindow::MenuSettingswindowSize()
{
}

void MainWindow::MenuSettingsFullscreen()
{
}

void MainWindow::MenuSettingsForceDMG()
{
}

void MainWindow::MenuDebugDisassembler()
{
}

void MainWindow::MenuDebugOAM()
{
}

void MainWindow::MenuDebugMap()
{
}

void MainWindow::MenuDebugPalette()
{
}

void MainWindow::MenuAbout()
{
}

void MainWindow::MenuPressed()
{
    m_pGLFrame->PauseRenderThread();
}

void MainWindow::MenuReleased()
{
    m_pGLFrame->ResumeRenderThread();
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    m_pGLFrame->StopRenderThread();
    QMainWindow::closeEvent(evt);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Up:
            m_pEmulator->KeyPressed(Up_Key);
            break;
        case Qt::Key_Left:
            m_pEmulator->KeyPressed(Left_Key);
            break;
        case Qt::Key_Right:
            m_pEmulator->KeyPressed(Right_Key);
            break;
        case Qt::Key_Down:
            m_pEmulator->KeyPressed(Down_Key);
            break;
        case Qt::Key_Return:
            m_pEmulator->KeyPressed(Start_Key);
            break;
        case Qt::Key_Space:
            m_pEmulator->KeyPressed(Select_Key);
            break;
        case Qt::Key_A:
            m_pEmulator->KeyPressed(B_Key);
            break;
        case Qt::Key_S:
            m_pEmulator->KeyPressed(A_Key);
            break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Up:
            m_pEmulator->KeyReleased(Up_Key);
            break;
        case Qt::Key_Left:
            m_pEmulator->KeyReleased(Left_Key);
            break;
        case Qt::Key_Right:
            m_pEmulator->KeyReleased(Right_Key);
            break;
        case Qt::Key_Down:
            m_pEmulator->KeyReleased(Down_Key);
            break;
        case Qt::Key_Return:
            m_pEmulator->KeyReleased(Start_Key);
            break;
        case Qt::Key_Space:
            m_pEmulator->KeyReleased(Select_Key);
            break;
        case Qt::Key_A:
            m_pEmulator->KeyReleased(B_Key);
            break;
        case Qt::Key_S:
            m_pEmulator->KeyReleased(A_Key);
            break;
    }
}
