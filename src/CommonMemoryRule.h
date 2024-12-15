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

#ifndef COMMONMEMORYRULE_H
#define	COMMONMEMORYRULE_H

#include "definitions.h"
#include "log.h"

class Memory;

class CommonMemoryRule
{
public:
    CommonMemoryRule(Memory* pMemory);
    ~CommonMemoryRule();
    u8 PerformRead(u16 address);
    void PerformWrite(u16 address, u8 value);
    void Reset(bool bCGB);

private:
    Memory* m_pMemory;
    bool m_bCGB;
};

#include "Memory.h"

inline u8 CommonMemoryRule::PerformRead(u16 address)
{
    if (m_bCGB)
    {
        switch (address & 0xE000)
        {
            case 0x8000:
            {
                return m_pMemory->ReadCGBLCDRAM(address, false);
            }
            case 0xC000:
            {
                return m_pMemory->ReadCGBWRAM(address);
            }
        }
    }
    else if (address >= 0xFEA0 && address < 0xFF00)
    {
        return ((((address + ((address >> 4) - 0x0FEA)) >> 2) & 1) ? 0x00 : 0xFF);
    }

    return m_pMemory->Retrieve(address);
}

inline void CommonMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x8000:
        {
            if (m_bCGB)
                m_pMemory->WriteCGBLCDRAM(address, value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xC000:
        {
            if (address < 0xDE00)
            {
                if (m_bCGB)
                    m_pMemory->WriteCGBWRAM(address, value);
                else
                    m_pMemory->Load(address, value);

                m_pMemory->Load(address + 0x2000, value);
            }
            else if (m_bCGB)
            {
                m_pMemory->WriteCGBWRAM(address, value);
            }
            else
            {
                m_pMemory->Load(address, value);
            }
            break;
        }
        case 0xE000:
        {
            if (address < 0xFE00)
            {
                if (m_bCGB)
                    m_pMemory->WriteCGBWRAM(address - 0x2000, value);
                else
                    m_pMemory->Load(address - 0x2000, value);

                m_pMemory->Load(address, value);
            }
            else
            {
                m_pMemory->Load(address, value);
            }
            break;
        }
        default:
        {
            Debug("--> ** Writing to invalid area %X %X", address, value);
            m_pMemory->Load(address, value);
        }
    }
}

#endif	/* COMMONMEMORYRULE_H */
