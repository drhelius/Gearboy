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

#ifndef MEMORYRULE_H
#define	MEMORYRULE_H

#include "definitions.h"
#include "TraceLogger.h"
#include "log.h"
#include <vector>

class Memory;
class Video;
class Processor;
class Input;
class Cartridge;
class Audio;

class MemoryRule
{
public:
    MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo,
            Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    void SetTraceLogger(TraceLogger* pTraceLogger);
    virtual ~MemoryRule();
    virtual u8 PerformRead(u16 address) = 0;
    virtual void PerformWrite(u16 address, u8 value) = 0;
    virtual bool MapsROMDirectly();
    virtual u8 GetMapperType();
    virtual bool NeedsHighMemoryAccessNotifications();
    virtual void NotifyHighMemoryRead(u16 address);
    virtual void NotifyHighMemoryWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB) = 0;
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
    virtual void SetRamChangedCallback(RamChangedCallback callback);
    virtual size_t GetRamSize();
    virtual size_t GetRTCSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual int GetCurrentRamBankIndex(u16 address);
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual u16 GetCurrentRomBankIndex(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address, u16 bank);
    virtual u8* GetRTCMemory();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

protected:
    INLINE void TraceMapperEvent(u16 address, u8 value, u8 event = 0xFF);
    INLINE bool IsTraceMapperEventEnabled(u8 event) const;
    NO_INLINE void LogTraceMapperEvent(u16 address, u8 value, u8 event, u8 flags, bool flags_valid);

    Processor* m_pProcessor;
    Memory* m_pMemory;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    Audio* m_pAudio;
    bool m_bCGB;
    RamChangedCallback m_pRamChangedCallback;
    TraceLogger* m_pTraceLogger;
};

INLINE void MemoryRule::TraceMapperEvent(u16 address, u8 value, u8 event)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (!m_pTraceLogger->IsEnabled(TRACE_MAPPER))
        return;

    if (event == 0xFF)
    {
        if (address < 0x2000 || address >= 0x6000)
            event = TRACE_MAPPER_CONTROL;
        else if (address < 0x4000)
            event = TRACE_MAPPER_ROM;
        else
            event = TRACE_MAPPER_RAM_RTC;
    }

    if (m_pTraceLogger->IsEventEnabled(TRACE_MAPPER, event))
        LogTraceMapperEvent(address, value, event, 0, false);
#else
    UNUSED(address);
    UNUSED(value);
    UNUSED(event);
#endif
}

INLINE bool MemoryRule::IsTraceMapperEventEnabled(u8 event) const
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    return m_pTraceLogger->IsEventEnabled(TRACE_MAPPER, event);
#else
    UNUSED(event);
    return false;
#endif
}

#endif	/* MEMORYRULE_H */
