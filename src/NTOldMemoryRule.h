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

#ifndef NTOLDMEMORYRULE_H
#define NTOLDMEMORYRULE_H

#include "MemoryRule.h"

class NTOldMemoryRule : public MemoryRule
{
public:
    NTOldMemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~NTOldMemoryRule();
    void SetVariant(bool type2);
    virtual u8 GetMapperType();
    virtual bool MapsROMDirectly();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream& stream);
    virtual bool LoadRam(std::istream& stream, s32 fileSize);
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    void SelectBank(u8 value);
    void UpdateBanks();

private:
    bool m_bType2;
    bool m_bSwapMode;
    bool m_bConfigLocked;
    bool m_bRamEnabled;
    bool m_bRumbleEnabled;
    bool m_bRumbleActive;
    u8 m_ROMBank;
    u8 m_ROMBase;
    u8 m_ROMBaseLatch;
    u8 m_ROMBankMask;
    int m_iCurrentROM0Bank;
    int m_iCurrentROMBank;
    u8 m_RAM[0x2000];
};

#endif /* NTOLDMEMORYRULE_H */
