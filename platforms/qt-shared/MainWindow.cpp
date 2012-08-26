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
#include <QDesktopWidget>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GLFrame.h"
#include "Emulator.h"
#include "InputSettings.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_bFullscreen = false;
    m_iScreenSize = 2;

    m_pInputSettings = new InputSettings();

    m_bMenuPressed[0] = m_bMenuPressed[1] = m_bMenuPressed[2] = false;
    m_pUI = new Ui::MainWindow();
    m_pUI->setupUi(this);

    this->addAction(m_pUI->actionFullscreen);
    this->addAction(m_pUI->actionReset);
    this->addAction(m_pUI->actionPause);
    this->addAction(m_pUI->actionSave_State);
    this->addAction(m_pUI->actionLoad_State);

    m_pExitShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_pExitShortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(m_pExitShortcut, SIGNAL(activated()), this, SLOT(Exit()));


    QObject::connect(m_pUI->menuGame_Boy, SIGNAL(aboutToShow()), this, SLOT(MenuGameBoyPressed()));
    QObject::connect(m_pUI->menuGame_Boy, SIGNAL(aboutToHide()), this, SLOT(MenuGameBoyReleased()));
    QObject::connect(m_pUI->menuDebug, SIGNAL(aboutToShow()), this, SLOT(MenuDebugPressed()));
    QObject::connect(m_pUI->menuDebug, SIGNAL(aboutToHide()), this, SLOT(MenuDebugReleased()));
    QObject::connect(m_pUI->menuSettings, SIGNAL(aboutToShow()), this, SLOT(MenuSettingsPressed()));
    QObject::connect(m_pUI->menuSettings, SIGNAL(aboutToHide()), this, SLOT(MenuSettingsReleased()));

    m_pUI->actionX_1->setData(1);
    m_pUI->actionX_2->setData(2);
    m_pUI->actionX_2->setChecked(true);
    m_pUI->actionX_3->setData(3);
    m_pUI->actionX_4->setData(4);
    m_pUI->actionX_5->setData(5);

    m_pEmulator = new Emulator();
    m_pEmulator->Init();

    QGLFormat f;
    f.setSwapInterval(1);
    QGLFormat::setDefaultFormat(f);

    m_pGLFrame = new GLFrame();
    ResizeWindow(m_iScreenSize);
    setCentralWidget(m_pGLFrame);

    //QPalette pal = this->palette();
    //pal.setColor(this->backgroundRole(), Qt::black);
    //this->setPalette(pal);

    m_pGLFrame->InitRenderThread(m_pEmulator);
}

MainWindow::~MainWindow()
{
    SafeDelete(m_pExitShortcut);
    SafeDelete(m_pEmulator);
    SafeDelete(m_pGLFrame);
    SafeDelete(m_pInputSettings);
    SafeDelete(m_pUI);
}

void MainWindow::Exit()
{
    this->close();
}

void MainWindow::MenuGameBoyLoadROM()
{
    m_pGLFrame->PauseRenderThread();

    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Load ROM"),
            QDir::currentPath(),
            tr("Game Boy ROM files (*.gb *.gbc *.sgb);;All files (*.*)"));

    if (!filename.isNull())
    {
        m_pEmulator->LoadRom(filename.toUtf8().data());
        m_pUI->actionPause->setChecked(false);
    }

    setFocus();
    activateWindow();

    m_pGLFrame->ResumeRenderThread();
}

void MainWindow::MenuGameBoyPause()
{
    if (m_pEmulator->IsPaused())
        m_pEmulator->Resume();
    else
        m_pEmulator->Pause();
}

void MainWindow::MenuGameBoyReset()
{
    m_pUI->actionPause->setChecked(false);
    m_pEmulator->Reset();
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
    m_pGLFrame->PauseRenderThread();
    m_pInputSettings->show();
    m_pGLFrame->ResumeRenderThread();
}

void MainWindow::MenuSettingsVideo()
{
}

void MainWindow::MenuSettingsSound()
{
}

void MainWindow::MenuSettingsWindowSize(QAction* action)
{
    m_pUI->actionX_1->setChecked(false);
    m_pUI->actionX_2->setChecked(false);
    m_pUI->actionX_3->setChecked(false);
    m_pUI->actionX_4->setChecked(false);
    m_pUI->actionX_5->setChecked(false);
    action->setChecked(true);
    m_iScreenSize = action->data().toInt();
    ResizeWindow(m_iScreenSize);
}

void MainWindow::MenuSettingsFullscreen()
{
    if (m_bFullscreen)
    {
        m_bFullscreen = false;
        this->showNormal();
        m_pUI->menubar->show();
        ResizeWindow(m_iScreenSize);
        m_pGLFrame->move(0, 0);
    }
    else
    {
        m_bFullscreen = true;

        this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        this->setMinimumSize(0, 0);
        this->showFullScreen();

        m_pUI->menubar->hide();

        int w = qApp->desktop()->size().width();
        int h = qApp->desktop()->size().height();

        int factor = h / GAMEBOY_HEIGHT;

        m_pGLFrame->setMaximumSize(GAMEBOY_WIDTH * factor, GAMEBOY_HEIGHT * factor);
        m_pGLFrame->setMinimumSize(GAMEBOY_WIDTH * factor, GAMEBOY_HEIGHT * factor);

        int move_x = (w - (GAMEBOY_WIDTH * factor)) / 2;
        int move_y = (h - (GAMEBOY_HEIGHT * factor)) / 2;
        m_pGLFrame->setGeometry(move_x, move_y, GAMEBOY_WIDTH * factor, GAMEBOY_HEIGHT * factor);
    }

    setFocus();
    activateWindow();
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

void MainWindow::MenuGameBoyPressed()
{
    m_bMenuPressed[0] = true;
    m_pGLFrame->PauseRenderThread();
}

void MainWindow::MenuGameBoyReleased()
{
    m_bMenuPressed[0] = false;
    MenuReleased();
}

void MainWindow::MenuSettingsPressed()
{
    m_bMenuPressed[1] = true;
    m_pGLFrame->PauseRenderThread();
}

void MainWindow::MenuSettingsReleased()
{
    m_bMenuPressed[1] = false;
    MenuReleased();
}

void MainWindow::MenuDebugPressed()
{
    m_bMenuPressed[2] = true;
    m_pGLFrame->PauseRenderThread();
}

void MainWindow::MenuDebugReleased()
{
    m_bMenuPressed[2] = false;
    MenuReleased();
}

void MainWindow::MenuReleased()
{
    for (int i = 0; i < 3; i++)
    {
        if (m_bMenuPressed[i])
            return;
    }
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

void MainWindow::ResizeWindow(int factor)
{
    m_iScreenSize = factor;
    m_pGLFrame->setMaximumSize(GAMEBOY_WIDTH * factor, GAMEBOY_HEIGHT * factor);
    m_pGLFrame->setMinimumSize(GAMEBOY_WIDTH * factor, GAMEBOY_HEIGHT * factor);
}

bool MainWindow::event(QEvent *ev)
{
    if (ev->type() == QEvent::LayoutRequest)
    {
        if (!m_bFullscreen)
        {
            this->setMaximumSize(sizeHint());
            this->setMinimumSize(sizeHint());
            this->resize(sizeHint());
        }
    }
    return QMainWindow::event(ev);
}

