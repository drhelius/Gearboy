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

#ifndef TAMA5MEMORYRULE_H
#define	TAMA5MEMORYRULE_H

#include "MemoryRule.h"

struct TAMA5_RTC
{
    u8 rtcTimerPage[16];
    u8 rtcAlarmPage[16];
    u8 rtcFreePage0[16];
    u8 rtcFreePage1[16];
    s32 lastTime;
    u8 disabled;
    u8 padding[3];
};

class TAMA5MemoryRule : public MemoryRule
{
public:
    TAMA5MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~TAMA5MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
    virtual size_t GetRamSize();
    virtual size_t GetRTCSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual u8* GetRTCMemory();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    void UpdateRTC();
    void ExecuteCommand();

private:
    int m_iCurrentROMBank;
    int m_CurrentROMAddress;
    u8 m_Reg;
    u8 m_Registers[8];
    u8 m_SRAM[32];
    TAMA5_RTC m_RTC;
};

#endif	/* TAMA5MEMORYRULE_H */
