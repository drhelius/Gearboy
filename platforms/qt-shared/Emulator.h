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

#ifndef EMULATOR_H
#define	EMULATOR_H

#include <QMutex>
#include "../../../src/gearboy.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();
    void Init();
    void RunToVBlank(GB_Color* pFrameBuffer);
    void LoadRom(const char* szFilePath, bool forceDMG);
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
    void Pause();
    void Resume();
    bool IsPaused();
    void Reset(bool forceDMG);
    void MemoryDump();
    void SetSoundSettings(bool enabled, int rate);
    void SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3, GB_Color& color4);
    void SaveRam();
    bool IsCGBRom();

private:
    GearboyCore* m_pGearboyCore;
    QMutex m_Mutex;
};

#endif	/* EMULATOR_H */

