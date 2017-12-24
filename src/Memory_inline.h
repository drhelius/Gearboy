#ifndef MEMORY_INLINE_H
#define	MEMORY_INLINE_H

#include "CommonMemoryRule.h"
#include "IORegistersMemoryRule.h"

inline u8 Memory::Read(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
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
    return m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)];
}

inline void Memory::WriteCGBWRAM(u16 address, u8 value)
{
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

inline bool Memory::IsDisassembled(u16 address)
{
    return m_pDisassembledMap[address].szDisString[0] != 0;
}

#endif	/* MEMORY_INLINE_H */
