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

#ifndef VIDEO_H
#define	VIDEO_H

#include "definitions.h"

class Memory;
class Processor;

class Video
{
public:
    Video(Memory* pMemory, Processor* pProcessor);
    ~Video();
    void Init();
    void Reset(bool bCGB);
    bool Tick(u8 clockCycles, u8* pFrameBuffer);
    void EnableScreen();
    void DisableScreen();
    bool IsScreenEnabled();
private:
    void ScanLine(int line);
    void RenderBG(int line);
    void RenderWindow(int line);
    void RenderSprites(int line);
    void UpdateStatRegister();
    void UpdateLYRegister();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    int* m_pSpriteXCacheBuffer;
    u8* m_pColorCacheBuffer;
    int m_iStatusMode;
    int m_iStatusModeCounter;
    int m_iStatusModeCounterAux;
    int m_iStatusModeLYCounter;
    int m_iScreenEnableDelayCycles;
    bool m_bScreenEnabled;
    bool m_bCGB;
};

#endif	/* VIDEO_H */

