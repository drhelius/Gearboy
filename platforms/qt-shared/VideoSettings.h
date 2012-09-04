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

#ifndef VIDEOSETTINGS_H
#define VIDEOSETTINGS_H

#include "ui_VideoSettings.h"

class GLFrame;
class Emulator;

class VideoSettings : public QDialog
{
    Q_OBJECT

public:
    VideoSettings(GLFrame* pGLFrame, Emulator* pEmulator);
    ~VideoSettings();
    void Init(int color1, int color2, int color3, int color4);

public slots:
    void PressedOK();
    void PressedCancel();
    void Color1();
    void Color2();
    void Color3();
    void Color4();

private:
    Ui::VideoSettings widget;
    Emulator* m_pEmulator;
    GLFrame* m_pGLFrame;
};

#endif // VIDEOSETTINGS_H
