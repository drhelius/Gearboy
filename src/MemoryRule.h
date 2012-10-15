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

#ifndef MEMORYRULE_H
#define	MEMORYRULE_H

#include "definitions.h"
#include <vector>

class Memory;
class Video;
class Processor;
class Input;
class Cartridge;
class Audio;

class MemoryRule
{
public:
    MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo,
            Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MemoryRule();
    virtual u8 PerformRead(u16 address) = 0;
    virtual void PerformWrite(u16 address, u8 value) = 0;
    virtual void Reset(bool bCGB) = 0;
    virtual void SaveRam(std::ofstream &file);
    virtual void LoadRam(std::ifstream &file);
    virtual int GetRamBanksSize();
    void AddAddressRange(u16 minAddress, u16 maxAddress);
    void ClearAddressRanges();
    bool IsAddressInRanges(u16 address);

protected:
    Processor* m_pProcessor;
    Memory* m_pMemory;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    Audio* m_pAudio;
    bool m_bCGB;

private:
    struct stAddressRange
    {
        u16 minAddress;
        u16 maxAddress;
    };
    std::vector<stAddressRange> m_Ranges;
    typedef std::vector<stAddressRange>::iterator AddressRangeVectorIterator;
};

#endif	/* MEMORYRULE_H */

