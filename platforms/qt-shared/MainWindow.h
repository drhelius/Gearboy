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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include "../../src/gearboy.h"

class GLFrame;
class Emulator;

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

public slots:
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
    void MenuSettingswindowSize();
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
    void MenuReleased();

protected:
    void closeEvent(QCloseEvent *evt);
    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

private:
    Ui::MainWindow *m_pUI;
    GLFrame *m_pGLFrame;
    Emulator* m_pEmulator;
    bool m_bMenuPressed[3];
};

#endif // MAINWINDOW_H
