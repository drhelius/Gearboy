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

#ifndef SINTAXMEMORYRULE_H
#define SINTAXMEMORYRULE_H

#include "MBC5LogoMemoryRule.h"

class SintaxMemoryRule : public MBC5LogoMemoryRule
{
public:
    SintaxMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~SintaxMemoryRule();
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
    void UpdateBank();

private:
    u8 m_BankSwapMode;
    u8 m_BankSelect;
    u16 m_RomBank;
    u8 m_Xor[4];
    int m_iCurrentROMBank;
    u8 m_BankSwap[16][256];
    u8 m_ROMView[0x4000];
};

#endif /* SINTAXMEMORYRULE_H */
