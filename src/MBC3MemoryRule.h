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
    
private:
    void UpdateRTC();
    
private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    u8* m_pRAMBanks;
    int m_iRTCSeconds;
    int m_iRTCMinutes;
    int m_iRTCHours;
    int m_iRTCDays;
    int m_iRTCControl;
    int m_iRTCLatchedSeconds;
    int m_iRTCLatchedMinutes;
    int m_iRTCLatchedHours;
    int m_iRTCLatchedDays;
    int m_iRTCLatchedControl;
    int m_iRTCLatch;
    u8 m_RTCRegister;
    time_t m_RTCLastTime;
    time_t m_RTCLastTimeCache;
};

#endif	/* MBC3MEMORYRULE_H */

