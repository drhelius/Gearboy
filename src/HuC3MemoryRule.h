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

#ifndef HUC3MEMORYRULE_H
#define	HUC3MEMORYRULE_H

#include "MemoryRule.h"

struct HuC3_RTC
{
    u16 minutes;
    u16 days;
    u16 alarm_minutes;
    u16 alarm_days;
    u8 alarm_enabled;
    s32 last_time;
    u8 padding[3];
};

class HuC3MemoryRule : public MemoryRule
{
public:
    HuC3MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~HuC3MemoryRule();
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
    bool HandleRTCWrite(u8 value);

private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    u8 m_iMode;
    u8* m_pRAMBanks;
    int m_CurrentROMAddress;
    int m_CurrentRAMAddress;
    HuC3_RTC m_RTC;
    u8 m_RTCAccessIndex;
    u8 m_RTCAccessFlags;
    u8 m_RTCReadValue;
};

#endif	/* HUC3MEMORYRULE_H */
