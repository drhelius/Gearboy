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

#ifndef HITEKMEMORYRULE_H
#define HITEKMEMORYRULE_H

#include "MBC5LogoMemoryRule.h"

class HitekMemoryRule : public MBC5LogoMemoryRule
{
public:
    HitekMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~HitekMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual u8 GetMapperType();
    virtual void Reset(bool bCGB);
    virtual size_t GetRamSize();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    u8 m_DataSwapMode;
    u8 m_BankSwapMode;
    int m_iCurrentROMBank;
    u8 m_DataSwap[8][256];
    u8 m_BankSwap[8][256];
    u8 m_ROMView[0x4000];
};

#endif /* HITEKMEMORYRULE_H */
