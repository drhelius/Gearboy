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

#ifndef NTNEWMEMORYRULE_H
#define NTNEWMEMORYRULE_H

#include "MBC5MemoryRule.h"

class NTNewMemoryRule : public MBC5MemoryRule
{
public:
    NTNewMemoryRule(Memory* pMemory, Cartridge* pCartridge);
    virtual ~NTNewMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual bool MapsROMDirectly();
    virtual u8 GetMapperType();
    virtual void Reset(bool bCGB);
    virtual size_t GetRamSize();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual u16 GetCurrentRomBankIndex(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address, u16 bank);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream, u32 version = GB_SAVESTATE_VERSION);

private:
    bool m_bSplitMode;
    u16 m_ROMBank;
    int m_iCurrentROMBankA;
    int m_iCurrentROMBankB;
    u8 m_ROMView[0x4000];
};

#endif /* NTNEWMEMORYRULE_H */
