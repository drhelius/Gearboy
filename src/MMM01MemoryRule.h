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

#ifndef MMM01MEMORYRULE_H
#define	MMM01MEMORYRULE_H

#include "MemoryRule.h"

class MMM01MemoryRule : public MemoryRule
{
public:
    MMM01MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MMM01MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
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
    void UpdateBanks();

private:
    bool m_bLocked;
    bool m_bRamEnabled;
    bool m_bMBC1Mode;
    bool m_bMBC1ModeDisable;
    bool m_bMultiplexMode;
    u8 m_RomBankLow;
    u8 m_RomBankMid;
    u8 m_RomBankHigh;
    u8 m_RomBankMask;
    u8 m_RamBankLow;
    u8 m_RamBankHigh;
    u8 m_RamBankMask;
    int m_iCurrentROMBank;
    int m_iCurrentROM0Bank;
    int m_iCurrentRAMBank;
    u8* m_pRAMBanks;
    int m_CurrentROMAddress;
    int m_CurrentROM0Address;
    int m_CurrentRAMAddress;
};

#endif	/* MMM01MEMORYRULE_H */
