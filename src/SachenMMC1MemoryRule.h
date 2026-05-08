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

#ifndef SACHENMMC1MEMORYRULE_H
#define SACHENMMC1MEMORYRULE_H

#include "MemoryRule.h"

class SachenMMC1MemoryRule : public MemoryRule
{
public:
    SachenMMC1MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~SachenMMC1MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    enum LockMode
    {
        LockModeDMG,
        LockModeCGB,
        LockModeUnlocked
    };

    u16 UnscrambleAddress(u16 address) const;
    int NormalizeROMBank(int bank) const;
    void SwitchROMBank0(int bank);
    void SwitchROMBank1(int bank);

private:
    LockMode m_LockMode;
    int m_iTransition;
    u8 m_Mask;
    u8 m_UnmaskedBank;
    u8 m_BaseBank;
    int m_iCurrentROM0Bank;
    int m_iCurrentROMBank;
    int m_CurrentROM0Address;
    int m_CurrentROMAddress;
};

#endif  /* SACHENMMC1MEMORYRULE_H */