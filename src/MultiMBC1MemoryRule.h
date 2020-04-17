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

#ifndef MULTIMBC1MEMORYRULE_H
#define	MULTIMBC1MEMORYRULE_H

#include "MemoryRule.h"

class MBC1MemoryRule;

class MultiMBC1MemoryRule : public MemoryRule
{
public:
    MultiMBC1MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio, MBC1MemoryRule* pMBC1MemoryRule);
    virtual ~MultiMBC1MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual u8* GetRomBank0();
    virtual u8* GetCurrentRomBank1();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    int m_iMulticartMode;
    MBC1MemoryRule* m_pMBC1MemoryRule;
    int m_iROMBankHi;
    int m_iROMBankLo;
};

#endif	/* MULTIMBC1MEMORYRULE_H */
