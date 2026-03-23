#ifndef MEMORY_INLINE_H
#define	MEMORY_INLINE_H

#include "CommonMemoryRule.h"
#include "IORegistersMemoryRule.h"

inline u8 Memory::Read(u16 address)
{
    #ifndef GEARBOY_DISABLE_DISASSEMBLER
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

            return m_pCurrentMemoryRule->PerformRead(address);
        }
        case 0x2000:
        case 0x4000:
        case 0x6000:
        {
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

inline u8 Memory::ReadCGBWRAM(u16 address)
{
    if (address < 0xD000)
        return m_pWRAMBanks[(address - 0xC000)];
    else
        return m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)];
}

inline void Memory::WriteCGBWRAM(u16 address, u8 value)
{
    if (address < 0xD000)
        m_pWRAMBanks[(address - 0xC000)] = value;
    else
        m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)] = value;
}

inline void Memory::SwitchCGBWRAM(u8 value)
{
    m_iCurrentWRAMBank = value;

    if (m_iCurrentWRAMBank == 0)
        m_iCurrentWRAMBank = 1;
}

inline u8 Memory::ReadCGBLCDRAM(u16 address, bool forceBank1)
{
    if (forceBank1 || (m_iCurrentLCDRAMBank == 1))
        return m_pLCDRAMBank1[address - 0x8000];
    else
        return Retrieve(address);
}

inline void Memory::WriteCGBLCDRAM(u16 address, u8 value)
{
    if (m_iCurrentLCDRAMBank == 1)
        m_pLCDRAMBank1[address - 0x8000] = value;
    else
        Load(address, value);
}

inline void Memory::SwitchCGBLCDRAM(u8 value)
{
    m_iCurrentLCDRAMBank = value;
}

inline u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address];
}

inline void Memory::Load(u16 address, u8 value)
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

    if (address < 0x4000)
    {
        int bank = m_pCurrentMemoryRule->GetCurrentRomBank0Index();
        return (u32)(0x4000 * bank) + address;
    }
    else
    {
        int bank = m_pCurrentMemoryRule->GetCurrentRomBank1Index();
        return (u32)(0x4000 * bank) + (address & 0x3FFF);
    }
}

inline u8 Memory::GetBank(u16 address)
{
    if (address >= 0x8000)
        return 0;

    if (!IsValidPointer(m_pCurrentMemoryRule))
        return 0;

    if (address < 0x4000)
        return (u8)m_pCurrentMemoryRule->GetCurrentRomBank0Index();
    else
        return (u8)m_pCurrentMemoryRule->GetCurrentRomBank1Index();
}

inline GB_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address)
{
    u32 physical_address = GetPhysicalAddress(address);
    bool rom = (address < 0x8000);

    if (rom)
    {
        if (physical_address >= MAX_ROM_SIZE)
            return NULL;
        return m_pDisassembledROMMap[physical_address];
    }
    else
    {
        return m_pDisassembledMap[physical_address];
    }
}

inline GB_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address, u8 bank)
{
    if (address >= 0x8000)
        return m_pDisassembledMap[address];

    u32 physical_address = (u32)(0x4000 * bank) + (address & 0x3FFF);
    if (physical_address >= MAX_ROM_SIZE)
        return NULL;
    return m_pDisassembledROMMap[physical_address];
}

inline GB_Disassembler_Record** Memory::GetAllDisassemblerRecords()
{
    return m_pDisassembledROMMap;
}

inline void Memory::CheckBreakpoints(u16 address, bool write)
{
    m_pProcessor->CheckMemoryBreakpoints(Processor::GB_BREAKPOINT_TYPE_ROMRAM, address, !write);
}

#endif	/* MEMORY_INLINE_H */
