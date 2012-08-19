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

#ifndef CORE_H
#define	CORE_H

#include "definitions.h"

class Memory;
class Processor;
class Video;
class Audio;
class Input;
class Cartridge;
class IORegistersMemoryRule;
class RomOnlyMemoryRule;
class MBC1MemoryRule;
class MBC2MemoryRule;
class MBC3MemoryRule;
class MBC5MemoryRule;

class GearboyCore
{
public:
    GearboyCore();
    ~GearboyCore();
    void Init();
    void RunToVBlank(GB_Color* pFrameBuffer);
    bool LoadROM(const char* szFilePath);
    Memory* GetMemory();
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
private:
    void InitMemoryRules();
    bool AddMemoryRules();
    void Reset(bool bCGB);
    void RenderDMGFrame(GB_Color* pFrameBuffer) const;
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Video* m_pVideo;
    Audio* m_pAudio;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    IORegistersMemoryRule* m_pIORegistersMemoryRule;
    RomOnlyMemoryRule* m_pRomOnlyMemoryRule;
    MBC1MemoryRule* m_pMBC1MemoryRule;
    MBC2MemoryRule* m_pMBC2MemoryRule;
    MBC3MemoryRule* m_pMBC3MemoryRule;
    MBC5MemoryRule* m_pMBC5MemoryRule;
    Gameboy_MemoryBankControllers m_MBC;
    bool m_bCGB;
};

#endif	/* CORE_H */

