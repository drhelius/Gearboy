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

#ifndef VF001MEMORYRULE_H
#define VF001MEMORYRULE_H

#include "MBC5LogoMemoryRule.h"

class VF001MemoryRule : public MBC5LogoMemoryRule
{
public:
    VF001MemoryRule(Memory* pMemory, Cartridge* pCartridge);
    virtual ~VF001MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual u8 DebugRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual u8 GetMapperType();
    virtual void Reset(bool bCGB);
    virtual size_t GetRamSize();
    virtual u8* GetRomBank0();
    virtual u16 GetCurrentRomBankIndex(u16 address);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream, u32 version = GB_SAVESTATE_VERSION);
    void SetInitialValue(u8 value);

private:
    u8 ReadROM(u16 address, bool debug);

    bool m_bConfigMode;
    u8 m_InitialValue;
    u8 m_RunningValue;
    u8 m_Registers[11];
    u8 m_ReplacementBankLatch;
    u8 m_SequenceBank;
    u16 m_SequenceAddress;
    u8 m_SequenceLength;
    u8 m_Sequence[4];
    u8 m_SequenceRemaining;
    bool m_bReplace;
    u16 m_ReplacementAddress;
    u8 m_ReplacementBank;
    u8 m_ROM0View[0x4000];
};

#endif /* VF001MEMORYRULE_H */
