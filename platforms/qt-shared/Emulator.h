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
    void LoadRom(const char* szFilePath);
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
    void Pause();
    void Resume();
    bool IsPaused();
    void Reset();
    
private:
    GearboyCore* m_pGearboyCore;
    QMutex m_Mutex;
};

#endif	/* EMULATOR_H */

