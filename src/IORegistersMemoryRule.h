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

#ifndef IOREGISTERSMEMORYRULE_H
#define	IOREGISTERSMEMORYRULE_H

#include "definitions.h"

class Memory;
class Video;
class Processor;
class Input;
class Cartridge;
class Audio;

class IORegistersMemoryRule
{
public:
    IORegistersMemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    ~IORegistersMemoryRule();
    u8 PerformRead(u16 address);
    void PerformWrite(u16 address, u8 value);
    void Reset(bool bCGB);
    
private:
    Processor* m_pProcessor;
    Memory* m_pMemory;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    Audio* m_pAudio;
    bool m_bCGB;
};

#endif	/* IOREGISTERSMEMORYRULE_H */

