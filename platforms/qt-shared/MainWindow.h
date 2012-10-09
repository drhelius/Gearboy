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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QShortcut>
#include "../../src/gearboy.h"

class GLFrame;
class Emulator;
class InputSettings;
class SoundSettings;
class VideoSettings;
class About;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool event(QEvent *ev);
    bool eventFilter(QObject * watched, QEvent * event);

public slots:
    void Exit();
    void MenuGameBoyLoadROM();
    void MenuGameBoyPause();
    void MenuGameBoyReset();
    void MenuGameBoySelectStateSlot();
    void MenuGameBoySaveState();
    void MenuGameBoyLoadState();
    void MenuGameBoySaveStateAs();
    void MenuGameBoyLoadStateFrom();
    void MenuSettingsInput();
    void MenuSettingsVideo();
    void MenuSettingsSound();
    void MenuSettingsWindowSize(QAction* action);
    void MenuSettingsFullscreen();
    void MenuSettingsForceDMG();
    void MenuDebugDisassembler();
    void MenuDebugOAM();
    void MenuDebugMap();
    void MenuDebugPalette();
    void MenuAbout();
    void MenuGameBoyPressed();
    void MenuGameBoyReleased();
    void MenuSettingsPressed();
    void MenuSettingsReleased();
    void MenuDebugPressed();
    void MenuDebugReleased();

protected:
    void closeEvent(QCloseEvent *evt);
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
    void MenuReleased();
    void ResizeWindow(int factor);

private:
    void LoadSettings();
    void SaveSettings();

private:
    Ui::MainWindow *m_pUI;
    GLFrame *m_pGLFrame;
    Emulator* m_pEmulator;
    bool m_bMenuPressed[3];
    int m_iScreenSize;
    bool m_bFullscreen;
    QShortcut* m_pExitShortcut;
    InputSettings* m_pInputSettings;
    SoundSettings* m_pSoundSettings;
    VideoSettings* m_pVideoSettings;
    About* m_pAbout;
};

#endif // MAINWINDOW_H
