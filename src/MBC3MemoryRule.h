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

#ifndef MBC3MEMORYRULE_H
#define	MBC3MEMORYRULE_H

#include "MemoryRule.h"

class MBC3MemoryRule : public MemoryRule
{
public:
    MBC3MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MBC3MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ofstream &file);
    virtual bool LoadRam(std::ifstream &file, s32 fileSize);

private:
    void UpdateRTC();

private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    bool m_bRTCEnabled;
    u8* m_pRAMBanks;
    s32 m_iRTCSeconds;
    s32 m_iRTCMinutes;
    s32 m_iRTCHours;
    s32 m_iRTCDays;
    s32 m_iRTCControl;
    s32 m_iRTCLatchedSeconds;
    s32 m_iRTCLatchedMinutes;
    s32 m_iRTCLatchedHours;
    s32 m_iRTCLatchedDays;
    s32 m_iRTCLatchedControl;
    s32 m_iRTCLatch;
    u8 m_RTCRegister;
    s32 m_RTCLastTime;
    s32 m_RTCLastTimeCache;
    int m_CurrentROMAddress;
    int m_CurrentRAMAddress;
};

#endif	/* MBC3MEMORYRULE_H */

