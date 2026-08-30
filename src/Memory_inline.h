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

#ifndef MEMORY_INLINE_H
#define	MEMORY_INLINE_H

#include "CommonMemoryRule.h"
#include "IORegistersMemoryRule.h"

INLINE void Memory::TraceLCDDMAEvent(u8 event, u16 source, u16 destination, u16 length)
{
    if (IsValidPointer(m_pTraceLogger) && m_pTraceLogger->IsEventEnabled(TRACE_LCD, event))
        LogLCDDMAEvent(event, source, destination, length);
}

inline u8 Memory::Read(u16 address)
{
    #ifndef GEARBOY_DISABLE_DISASSEMBLER
    if (unlikely(m_pProcessor->MemoryBreakpointsEnabled()))
        CheckBreakpoints(address, false);
    #endif

    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (!m_bBootromRegistryDisabled)
            {
                if (m_bCGB)
                {
                    if (m_bBootromGBCEnabled && m_bBootromGBCLoaded && ((address < 0x0100) || (address < 0x0900 && address > 0x01FF)))
                        return m_pBootromGBC[address];
                }
                else
                {
                    if (m_bBootromDMGEnabled && m_bBootromDMGLoaded && (address < 0x0100))
                        return m_pBootromDMG[address];
                }
            }

            if (likely(IsValidPointer(m_pDirectROMPages[0])))
                return m_pDirectROMPages[0][address];

            return m_pCurrentMemoryRule->PerformRead(address);
        }
        case 0x2000:
        {
            if (likely(IsValidPointer(m_pDirectROMPages[0])))
                return m_pDirectROMPages[0][address];

            return m_pCurrentMemoryRule->PerformRead(address);
        }
        case 0x4000:
        case 0x6000:
        {
            if (likely(IsValidPointer(m_pDirectROMPages[1])))
                return m_pDirectROMPages[1][address & 0x3FFF];

            return m_pCurrentMemoryRule->PerformRead(address);
        }
        case 0x8000:
        {
            return m_pCommonMemoryRule->PerformRead(address);
        }
        case 0xA000:
        {
            return m_pCurrentMemoryRule->PerformRead(address);
        }
        case 0xC000:
        case 0xE000:
        {
            if (m_bCurrentRuleNeedsHighMemoryAccessNotifications)
                m_pCurrentMemoryRule->NotifyHighMemoryRead(address);

            if (address < 0xFF00)
                return m_pCommonMemoryRule->PerformRead(address);
            else
                return m_pIORegistersMemoryRule->PerformRead(address);
        }
        default:
        {
            return Retrieve(address);
        }
    }
}

inline void Memory::Write(u16 address, u8 value)
{
    #ifndef GEARBOY_DISABLE_DISASSEMBLER
    if (unlikely(m_pProcessor->MemoryBreakpointsEnabled()))
        CheckBreakpoints(address, true);
    #endif

    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        case 0x4000:
        case 0x6000:
        {
            m_pCurrentMemoryRule->PerformWrite(address, value);
            if (m_bCurrentRuleMapsROMDirectly)
                RefreshDirectROMPages();
            break;
        }
        case 0x8000:
        {
            m_pCommonMemoryRule->PerformWrite(address, value);
            break;
        }
        case 0xA000:
        {
            m_pCurrentMemoryRule->PerformWrite(address, value);
            break;
        }
        case 0xC000:
        case 0xE000:
        {
            if (m_bCurrentRuleNeedsHighMemoryAccessNotifications)
                m_pCurrentMemoryRule->NotifyHighMemoryWrite(address, value);

            if (address < 0xFF00)
                m_pCommonMemoryRule->PerformWrite(address, value);
            else
                m_pIORegistersMemoryRule->PerformWrite(address, value);
            break;
        }
        default:
        {
            Load(address, value);
            break;
        }
    }
}

INLINE u8 Memory::ReadCGBWRAM(u16 address)
{
    if (address < 0xD000)
        return m_pWRAMBanks[(address - 0xC000)];
    else
        return m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)];
}

INLINE void Memory::WriteCGBWRAM(u16 address, u8 value)
{
    if (address < 0xD000)
        m_pWRAMBanks[(address - 0xC000)] = value;
    else
        m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)] = value;
}

INLINE void Memory::SwitchCGBWRAM(u8 value)
{
    m_iCurrentWRAMBank = value & 0x07;

    if (m_iCurrentWRAMBank == 0)
        m_iCurrentWRAMBank = 1;
}

INLINE u8 Memory::ReadCGBLCDRAM(u16 address, bool forceBank1)
{
    if (forceBank1 || (m_iCurrentLCDRAMBank == 1))
        return m_pLCDRAMBank1[address - 0x8000];
    else
        return Retrieve(address);
}

INLINE void Memory::WriteCGBLCDRAM(u16 address, u8 value)
{
    if (m_iCurrentLCDRAMBank == 1)
        m_pLCDRAMBank1[address - 0x8000] = value;
    else
        Load(address, value);
}

INLINE void Memory::SwitchCGBLCDRAM(u8 value)
{
    m_iCurrentLCDRAMBank = value;
}

INLINE u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address];
}

INLINE void Memory::Load(u16 address, u8 value)
{
    m_pMap[address] = value;
}

inline u8 Memory::DebugRetrieve(u16 address)
{
    if (address < 0x8000)
    {
        if (!m_bBootromRegistryDisabled)
        {
            if (m_bCGB)
            {
                if (m_bBootromGBCEnabled && m_bBootromGBCLoaded && ((address < 0x0100) || (address < 0x0900 && address > 0x01FF)))
                    return m_pBootromGBC[address];
            }
            else
            {
                if (m_bBootromDMGEnabled && m_bBootromDMGLoaded && (address < 0x0100))
                    return m_pBootromDMG[address];
            }
        }
        if (IsValidPointer(m_pCurrentMemoryRule))
            return m_pCurrentMemoryRule->PerformRead(address);
    }
    return m_pMap[address];
}

inline u32 Memory::GetPhysicalAddress(u16 address)
{
    if (address >= 0x8000)
        return (u32)address;

    if (!IsValidPointer(m_pCurrentMemoryRule))
        return (u32)address;

    return m_pCurrentMemoryRule->GetPhysicalROMAddress(address);
}

inline u8 Memory::GetBank(u16 address)
{
    if (address >= 0x8000)
        return 0;

    if (!IsValidPointer(m_pCurrentMemoryRule))
        return 0;

    return (u8)m_pCurrentMemoryRule->GetCurrentRomBankIndex(address);
}

inline u16 Memory::GetTraceBank(u16 address)
{
    if (address >= 0x8000)
        return 0;

    if (!IsValidPointer(m_pCurrentMemoryRule))
        return 0;

    return m_pCurrentMemoryRule->GetCurrentRomBankIndex(address);
}

inline GB_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address)
{
    u32 physical_address = GetPhysicalAddress(address);
    bool rom = (address < 0x8000);

    if (rom)
    {
        if (physical_address >= MAX_ROM_DISASSEMBLY_SIZE)
            return NULL;
        return m_pDisassembledROMMap[physical_address];
    }
    else
    {
        return m_pDisassembledMap[physical_address];
    }
}

inline GB_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address, u16 bank)
{
    if (address >= 0x8000)
        return m_pDisassembledMap[address];

    u32 physical_address = m_pCurrentMemoryRule->GetPhysicalROMAddress(address, bank);
    if (physical_address >= MAX_ROM_DISASSEMBLY_SIZE)
        return NULL;
    return m_pDisassembledROMMap[physical_address];
}

inline GB_Disassembler_Record** Memory::GetAllDisassemblerRecords()
{
    return m_pDisassembledROMMap;
}

INLINE bool Memory::IsVRAMAccessBlocked() const
{
    return IsValidPointer(m_pVideo) && m_pVideo->VRAMAccessBlocked();
}

INLINE bool Memory::IsHDMAEnabled() const
{
    return m_bHDMAEnabled;
}

#endif	/* MEMORY_INLINE_H */
