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

#ifndef _INPUTSETTINGS_H
#define	_INPUTSETTINGS_H

#include <QSettings>
#include "ui_InputSettings.h"

struct stCustomKey
{
    int keyCode;
    char text[32];
};

class GLFrame;

class InputSettings : public QDialog
{
    Q_OBJECT

public:
    InputSettings(GLFrame* pGLFrame);
    ~InputSettings();
    bool eventFilter(QObject* pObj, QEvent *pEvent);
    int GetKey(int key);
    void SaveSettings(QSettings& settings);
    void LoadSettings(QSettings& settings);

public slots:
    void SaveKeys();
    void RestoreKeys();

private:
    void PrintKey(QKeyEvent& pEvent, char* buffer);

private:
    Ui::InputSettings widget;
    stCustomKey m_Keys[8];
    stCustomKey m_TempKeys[8];
    GLFrame* m_pGLFrame;
};

#endif	/* _INPUTSETTINGS_H */
