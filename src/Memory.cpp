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

#include <iostream>
#include <fstream>
#include "Memory.h"
#include "MemoryRule.h"
#include "Processor.h"
#include "Video.h"

Memory::Memory()
{
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pMap);
    InitPointer(m_pDisassembledMap);
    InitPointer(m_pWRAMBanks);
    InitPointer(m_pLCDRAMBank1);
    m_bCGB = false;
    m_iCurrentWRAMBank = 1;
    m_iCurrentLCDRAMBank = 0;
    m_bHDMAEnabled = false;
    m_iHDMABytes = 0;
    for (int i = 0; i < 5; i++)
        m_HDMA[i] = 0;
    m_HDMASource = 0;
    m_HDMADestination = 0;
}

Memory::~Memory()
{
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    SafeDeleteArray(m_pMap);
    SafeDeleteArray(m_pDisassembledMap);
    SafeDeleteArray(m_pWRAMBanks);
    SafeDeleteArray(m_pLCDRAMBank1);
}

void Memory::SetProcessor(Processor* pProcessor)
{
    m_pProcessor = pProcessor;
}

void Memory::SetVideo(Video* pVideo)
{
    m_pVideo = pVideo;
}

void Memory::Init()
{
    m_pMap = new u8[65536];
    m_pWRAMBanks = new u8[0x8000];
    m_pLCDRAMBank1 = new u8[0x2000];
    m_pDisassembledMap = new stDisassemble[65536];
    Reset(false);
}

void Memory::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_Rules.clear();
    m_iCurrentWRAMBank = 1;
    m_iCurrentLCDRAMBank = 0;
    m_bHDMAEnabled = false;
    m_iHDMABytes = 0;

    for (int i = 0; i < 65536; i++)
    {
        m_pMap[i] = 0x00;
        m_pDisassembledMap[i].szDisString[0] = 0;

        if ((i >= 0x8000) && (i < 0xA000))
        {
            m_pMap[i] = 0x00;
            m_pLCDRAMBank1[i - 0x8000] = 0x00;
        }
        else if ((i >= 0xC000) && (i < 0xE000))
        {
            if ((i & 0x8) ^((i & 0x800) >> 8))
            {
                if (m_bCGB)
                {
                    m_pMap[i] = 0x00;
                    if (i >= 0xD000)
                    {
                        for (int a = 0; a < 8; a++)
                        {
                            if (a != 2)
                                m_pWRAMBanks[(i - 0xD000) + (0x1000 * a)] = m_pMap[i - 0x1000];
                            else
                                m_pWRAMBanks[(i - 0xD000) + (0x1000 * a)] = 0x00;
                        }
                    }
                }
                else
                    m_pMap[i] = 0x0f;
            }
            else
            {
                m_pMap[i] = 0xff;
                if (i >= 0xD000)
                {
                    for (int a = 0; a < 8; a++)
                    {
                        if (a != 2)
                            m_pWRAMBanks[(i - 0xD000) + (0x1000 * a)] = m_pMap[i - 0x1000];
                        else
                            m_pWRAMBanks[(i - 0xD000) + (0x1000 * a)] = 0x00;
                    }
                }
            }
        }
        else if (i >= 0xFF00)
        {
            if (m_bCGB)
                m_pMap[i] = kInitialValuesForColorFFXX[i - 0xFF00];
            else
                m_pMap[i] = kInitialValuesForFFXX[i - 0xFF00];
        }
        else
        {
            m_pMap[i] = 0xFF;
        }
    }

    if (m_bCGB)
    {
        for (int i = 0; i < 5; i++)
        {
            m_HDMA[i] = m_pMap[0xFF51 + i];
        }

        u8 hdma1 = m_HDMA[0];
        u8 hdma2 = m_HDMA[1];
        u8 hdma3 = m_HDMA[2];
        u8 hdma4 = m_HDMA[3];

        if (hdma1 > 0x7f && hdma1 < 0xa0)
            hdma1 = 0;

        m_HDMASource = (hdma1 << 8) | (hdma2 & 0xF0);
        m_HDMADestination = ((hdma3 & 0x1F) << 8) | (hdma4 & 0xF0);
        m_HDMADestination |= 0x8000;
    }
}

void Memory::AddRule(MemoryRule* pRule)
{
    m_Rules.push_back(pRule);
}

u8 Memory::Read(u16 address)
{
    RulesVectorIterator it;
    RulesVectorIterator end = m_Rules.end();
    MemoryRule* pRule = NULL;

    for (it = m_Rules.begin(); it < end; it++)
    {
        pRule = *it;
        if (pRule->IsAddressInRanges(address))
        {
            return pRule->PerformRead(address);
        }
    }

    return Retrieve(address);
}

void Memory::Write(u16 address, u8 value)
{
    RulesVectorIterator it;
    RulesVectorIterator end = m_Rules.end();
    MemoryRule* pRule = NULL;

    for (it = m_Rules.begin(); it < end; it++)
    {
        pRule = *it;
        if (pRule->IsAddressInRanges(address))
        {
            pRule->PerformWrite(address, value);
            return;
        }
    }

    Load(address, value);
}

u8 Memory::ReadCGBWRAM(u16 address)
{
    return m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)];
}

void Memory::WriteCGBWRAM(u16 address, u8 value)
{
    m_pWRAMBanks[(address - 0xD000) + (0x1000 * m_iCurrentWRAMBank)] = value;
}

void Memory::SwitchCGBWRAM(u8 value)
{
    m_iCurrentWRAMBank = value;

    if (m_iCurrentWRAMBank == 0)
        m_iCurrentWRAMBank = 1;
}

u8 Memory::ReadCGBLCDRAM(u16 address, bool forceBank1)
{
    if (forceBank1 || (m_iCurrentLCDRAMBank == 1))
        return m_pLCDRAMBank1[address - 0x8000];
    else
        return Retrieve(address);
}

void Memory::WriteCGBLCDRAM(u16 address, u8 value)
{
    if (m_iCurrentLCDRAMBank == 1)
        m_pLCDRAMBank1[address - 0x8000] = value;
    else
        Load(address, value);
}

void Memory::SwitchCGBLCDRAM(u8 value)
{
    m_iCurrentLCDRAMBank = value;
}

u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address];
}

void Memory::Load(u16 address, u8 value)
{
    m_pMap[address] = value;
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    strcpy(m_pDisassembledMap[address].szDisString, szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pDisassembledMap[address].szDisString[0] != 0;
}

void Memory::LoadBank0and1FromROM(u8* pTheROM)
{
    // loads the first 32KB only (bank 0 and 1)
    for (int i = 0; i < 0x8000; i++)
    {
        m_pMap[i] = pTheROM[i];
    }
}

void Memory::MemoryDump(const char* szFilePath)
{
    using namespace std;

    ofstream myfile(szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 65536; i++)
        {
            if (IsDisassembled(i))
            {
                myfile << "0x" << hex << i << "\t " << m_pDisassembledMap[i].szDisString << "\n";
            }
            else
            {
                myfile << "0x" << hex << i << "\t [0x" << hex << (int) m_pMap[i] << "]\n";
            }
        }

        myfile.close();
    }
}

void Memory::PerformDMA(u8 value)
{
    if (m_bCGB)
    {
        u16 address = value << 8;
        if (address < 0xE000)
        {
            if (address >= 0x8000 && address < 0xA000)
            {
                for (int i = 0; i < 0xA0; i++)
                    Load(0xFE00 + i, ReadCGBLCDRAM(address + i, false));
            }
            else if (address >= 0xD000 && address < 0xE000)
            {
                for (int i = 0; i < 0xA0; i++)
                    Load(0xFE00 + i, ReadCGBWRAM(address + i));
            }
            else
            {
                for (int i = 0; i < 0xA0; i++)
                    Load(0xFE00 + i, Read(address + i));
            }
        }
    }
    else
    {
        u16 address = value << 8;
        if (address >= 0x8000 && address < 0xE000)
        {
            for (int i = 0; i < 0xA0; i++)
                Load(0xFE00 + i, Read(address + i));
        }
    }
}

void Memory::SwitchCGBDMA(u8 value)
{
    m_iHDMABytes = 16 + ((value & 0x7f) * 16);

    if (m_bHDMAEnabled)
    {
        if (IsSetBit(value, 7))
        {
            m_HDMA[4] = value & 0x7F;
        }
        else
        {
            m_HDMA[4] = 0xFF;
            m_bHDMAEnabled = false;
        }
    }
    else
    {
        if (IsSetBit(value, 7))
        {
            m_bHDMAEnabled = true;
            m_HDMA[4] = value & 0x7F;
            if (m_pVideo->GetCurrentStatusMode() == 0)
            {
                m_pProcessor->AddCycles(PerformHDMA());
            }
        }
        else
        {
            PerformGDMA(value);
        }
    }
}

unsigned int Memory::PerformHDMA()
{
    u16 source = m_HDMASource & 0xFFF0;
    u16 destination = (m_HDMADestination & 0x1FF0) | 0x8000;

    if (source >= 0xD000 && source < 0xE000)
    {
        for (int i = 0; i < 0x10; i++)
            WriteCGBLCDRAM(destination + i, ReadCGBWRAM(source + i));
    }
    else
    {
        for (int i = 0; i < 0x10; i++)
            WriteCGBLCDRAM(destination + i, Read(source + i));
    }

    m_HDMADestination += 0x10;
    if (m_HDMADestination == 0xA000)
        m_HDMADestination = 0x8000;

    m_HDMASource += 0x10;
    if (m_HDMASource == 0x8000)
        m_HDMASource = 0xA000;

    m_HDMA[1] = m_HDMASource & 0xFF;
    m_HDMA[0] = m_HDMASource >> 8;

    m_HDMA[3] = m_HDMADestination & 0xFF;
    m_HDMA[2] = m_HDMADestination >> 8;

    m_iHDMABytes -= 0x10;
    m_HDMA[4]--;

    if (m_HDMA[4] == 0xFF)
        m_bHDMAEnabled = false;

    // return clock cycles used
    return (m_pProcessor->CGBSpeed() ? 17 : 9) * 4;
}

void Memory::PerformGDMA(u8 value)
{
    u16 source = m_HDMASource & 0xFFF0;
    u16 destination = (m_HDMADestination & 0x1FF0) | 0x8000;

    if (source >= 0xD000 && source < 0xE000)
    {
        for (int i = 0; i < m_iHDMABytes; i++)
            WriteCGBLCDRAM(destination + i, ReadCGBWRAM(source + i));
    }
    else
    {
        for (int i = 0; i < m_iHDMABytes; i++)
            WriteCGBLCDRAM(destination + i, Read(source + i));
    }

    m_HDMADestination += m_iHDMABytes;
    m_HDMASource += m_iHDMABytes;

    for (int i = 0; i < 5; i++)
        m_HDMA[i] = 0xFF;

    int clock_cycles = 0;

    if (m_pProcessor->CGBSpeed())
        clock_cycles = 2 + 16 * ((value & 0x7f) + 1);
    else
        clock_cycles = 1 + 8 * ((value & 0x7f) + 1);

    m_pProcessor->AddCycles(clock_cycles * 4);
}

bool Memory::IsHDMAEnabled()
{
    return m_bHDMAEnabled;
}

void Memory::SetHDMARegister(int reg, u8 value)
{
    switch (reg)
    {
        case 1:
        {
            // HDMA1
            if (value > 0x7f && value < 0xa0)
                value = 0;
            m_HDMASource = (value << 8) | (m_HDMASource & 0xF0);
            break;
        }
        case 2:
        {
            // HDMA2
            value &= 0xF0;
            m_HDMASource = (m_HDMASource & 0xFF00) | value;
            break;
        }
        case 3:
        {
            // HDMA3
            value &= 0x1F;
            m_HDMADestination = (value << 8) | (m_HDMADestination & 0xF0);
            m_HDMADestination |= 0x8000;
            break;
        }
        case 4:
        {
            // HDMA4
            value &= 0xF0;
            m_HDMADestination = (m_HDMADestination & 0x1F00) | value;
            m_HDMADestination |= 0x8000;
            break;
        }
    }

    m_HDMA[reg - 1] = value;
}

u8 Memory::GetHDMARegister(int reg)
{
    return m_HDMA[reg - 1];
}

