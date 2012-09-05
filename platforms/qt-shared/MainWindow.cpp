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

#include <QFileDialog>
#include <QDesktopWidget>
#include <QSettings>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GLFrame.h"
#include "Emulator.h"
#include "InputSettings.h"
#include "SoundSettings.h"
#include "VideoSettings.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_bFullscreen = false;
    m_iScreenSize = 2;

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

    m_pInputSettings = new InputSettings(m_pGLFrame);
    InitKeys();

    m_pSoundSettings = new SoundSettings(m_pGLFrame, m_pEmulator);
    m_pSoundSettings->Init(true, 1);

    m_pVideoSettings = new VideoSettings(m_pGLFrame, m_pEmulator);
    m_pVideoSettings->Init(0xEFF3D5, 0xA3B67A, 0x37613B, 0x041C16);

    //QPalette pal = this->palette();
    //pal.setColor(this->backgroundRole(), Qt::black);
    //this->setPalette(pal);

    LoadSettings();

    m_pGLFrame->InitRenderThread(m_pEmulator);
}

MainWindow::~MainWindow()
{
    SaveSettings();

    SafeDelete(m_pExitShortcut);
    SafeDelete(m_pEmulator);
    SafeDelete(m_pGLFrame);
    SafeDelete(m_pInputSettings);
    SafeDelete(m_pSoundSettings);
    SafeDelete(m_pUI);
}

void MainWindow::Exit()
{
#ifdef DEBUG_GEARBOY
    m_pEmulator->MemoryDump();
#endif
    this->close();
}

void MainWindow::MenuGameBoyLoadROM()
{
    m_pGLFrame->PauseRenderThread();

    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Load ROM"),
            QDir::currentPath(),
            tr("Game Boy ROM files (*.gb *.dmg *.gbc *.cgb *.sgb);;All files (*.*)"));

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
}

void MainWindow::MenuSettingsVideo()
{
    m_pGLFrame->PauseRenderThread();
    m_pVideoSettings->show();
}

void MainWindow::MenuSettingsSound()
{
    m_pGLFrame->PauseRenderThread();
    m_pSoundSettings->show();
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
    switch (m_pInputSettings->GetKey(e->key()))
    {
        case 0:
            m_pEmulator->KeyPressed(Up_Key);
            break;
        case 3:
            m_pEmulator->KeyPressed(Left_Key);
            break;
        case 1:
            m_pEmulator->KeyPressed(Right_Key);
            break;
        case 2:
            m_pEmulator->KeyPressed(Down_Key);
            break;
        case 6:
            m_pEmulator->KeyPressed(Start_Key);
            break;
        case 7:
            m_pEmulator->KeyPressed(Select_Key);
            break;
        case 5:
            m_pEmulator->KeyPressed(B_Key);
            break;
        case 4:
            m_pEmulator->KeyPressed(A_Key);
            break;
        default:
            break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* e)
{
    switch (m_pInputSettings->GetKey(e->key()))
    {
        case 0:
            m_pEmulator->KeyReleased(Up_Key);
            break;
        case 3:
            m_pEmulator->KeyReleased(Left_Key);
            break;
        case 1:
            m_pEmulator->KeyReleased(Right_Key);
            break;
        case 2:
            m_pEmulator->KeyReleased(Down_Key);
            break;
        case 6:
            m_pEmulator->KeyReleased(Start_Key);
            break;
        case 7:
            m_pEmulator->KeyReleased(Select_Key);
            break;
        case 5:
            m_pEmulator->KeyReleased(B_Key);
            break;
        case 4:
            m_pEmulator->KeyReleased(A_Key);
            break;
        default:
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

void MainWindow::InitKeys()
{
    stCustomKey keys[8];

    keys[0].keyCode = Qt::Key_Up;
    strcpy(keys[0].text, "UP");
    keys[1].keyCode = Qt::Key_Right;
    strcpy(keys[1].text, "RIGHT");
    keys[2].keyCode = Qt::Key_Down;
    strcpy(keys[2].text, "DOWN");
    keys[3].keyCode = Qt::Key_Left;
    strcpy(keys[3].text, "LEFT");
    keys[4].keyCode = Qt::Key_S;
    strcpy(keys[4].text, "S");
    keys[5].keyCode = Qt::Key_A;
    strcpy(keys[5].text, "A");
    keys[6].keyCode = Qt::Key_Return;
    strcpy(keys[6].text, "RETURN");
    keys[7].keyCode = Qt::Key_Space;
    strcpy(keys[7].text, "SPACE");

    m_pInputSettings->SetKeys(keys);
}

void MainWindow::LoadSettings()
{
    QSettings settings("gearboy.ini", QSettings::IniFormat);
    m_iScreenSize = settings.value("ScreenSize", 2).toInt();

    switch (m_iScreenSize)
    {
        case 1:
            MenuSettingsWindowSize(m_pUI->actionX_1);
            break;
        case 2:
            MenuSettingsWindowSize(m_pUI->actionX_2);
            break;
        case 3:
            MenuSettingsWindowSize(m_pUI->actionX_3);
            break;
        case 4:
            MenuSettingsWindowSize(m_pUI->actionX_4);
            break;
        case 5:
            MenuSettingsWindowSize(m_pUI->actionX_5);
            break;
    }

    m_bFullscreen = !settings.value("FullScreen", false).toBool();

    MenuSettingsFullscreen();

    m_pUI->actionForce_Game_Boy_DMG->setChecked(settings.value("ForceDMG", false).toBool());

    m_pInputSettings->LoadSettings(settings);
    m_pVideoSettings->LoadSettings(settings);
    m_pSoundSettings->LoadSettings(settings);
}

void MainWindow::SaveSettings()
{
    QSettings settings("gearboy.ini", QSettings::IniFormat);
    settings.setValue("ScreenSize", m_iScreenSize);
    settings.setValue("FullScreen", m_bFullscreen);
    settings.setValue("ForceDMG", m_pUI->actionForce_Game_Boy_DMG->isChecked());

    m_pInputSettings->SaveSettings(settings);
    m_pVideoSettings->SaveSettings(settings);
    m_pSoundSettings->SaveSettings(settings);
}

