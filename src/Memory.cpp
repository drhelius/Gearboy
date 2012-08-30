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
#include "MemoryCell.h"

Memory::Memory()
{
    InitPointer(m_pMap);
    InitPointer(m_pWRAMBanks);
    InitPointer(m_pLCDRAMBank1);
    m_bCGB = false;
    m_iCurrentWRAMBank = 1;
    m_iCurrentLCDRAMBank = 0;
    m_bHBDMAEnabled = false;
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
    SafeDeleteArray(m_pWRAMBanks);
    SafeDeleteArray(m_pLCDRAMBank1);
}

void Memory::Init()
{
    m_pMap = new MemoryCell[65536];
    m_pWRAMBanks = new u8[0x8000];
    m_pLCDRAMBank1 = new u8[0x2000];
    Reset(false);
}

void Memory::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_Rules.clear();
    m_iCurrentWRAMBank = 1;
    m_iCurrentLCDRAMBank = 0;
    m_bHBDMAEnabled = false;

    for (int i = 0; i < 65536; i++)
    {
        m_pMap[i].Reset();

		if ((i >= 0xA000) && (i < 0xC000))
		{
			m_pMap[i].SetValue(0xFF);
		}
        else if (i >= 0xFF00)
		{
            m_pMap[i].SetValue(kInitialValuesForFFXX[i - 0xFF00]);
		}
    }
}

void Memory::AddRule(MemoryRule* pRule)
{
    m_Rules.push_back(pRule);
}

u8 Memory::Read(u16 address)
{
    RulesVectorIterator it;

    for (it = m_Rules.begin(); it < m_Rules.end(); it++)
    {
        MemoryRule* pRule = *it;
        if (pRule->IsEnabled() && pRule->IsAddressInRange(address))
        {
            return pRule->PerformRead(address);
        }
    }

    return Retrieve(address);
}

void Memory::Write(u16 address, u8 value)
{
    RulesVectorIterator it;

    bool ruleExecuted = false;

    for (it = m_Rules.begin(); it < m_Rules.end(); it++)
    {
        MemoryRule* pRule = *it;
        if (pRule->IsEnabled() && pRule->IsAddressInRange(address))
        {
            pRule->PerformWrite(address, value);
            ruleExecuted = true;
            break;
        }
    }

    if (!ruleExecuted)
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
    m_iCurrentWRAMBank = value & 0x07;

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
    m_iCurrentLCDRAMBank = value & 0x01;
}

u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address].Read();
}

void Memory::Load(u16 address, u8 value)
{
    m_pMap[address].Write(value);
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    m_pMap[address].SetDisassembledString(szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pMap[address].GetDisassembledString()[0] != 0;
}

void Memory::LoadBank0and1FromROM(u8* pTheROM)
{
    // loads the first 32KB only (bank 0 and 1)
    for (int i = 0; i < 0x8000; i++)
    {
        m_pMap[i].SetValue(pTheROM[i]);
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
                myfile << "0x" << hex << i << "\t " << m_pMap[i].GetDisassembledString() << "\n";
            }
            else
            {
                myfile << "0x" << hex << i << "\t [0x" << hex << (int) m_pMap[i].GetValue() << "]\n";
            }
        }

        myfile.close();
    }
}

void Memory::DoDMATransfer(u8 value)
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

void Memory::DoDMACGBTransfer(u8 value, bool hbdma)
{
    if (hbdma)
    {
        // Horizontal Blanking DMA
        m_bHBDMAEnabled = IsSetBit(value, 7);
    }
    else
    {
        // General purpose DMA
        DoHDMACGBTransfer(false);
        // transfer finished
        Load(0xFF51, 0xFF);
        Load(0xFF52, 0xFF);
        Load(0xFF53, 0xFF);
        Load(0xFF54, 0xFF);
        Load(0xFF55, 0xFF);
    }
}

void Memory::DoHDMACGBTransfer(bool hbdma)
{
    u8 hdma5 = Retrieve(0xFF55);

    int bytes = 16 + (hdma5 & 0x7F) * 16;

    if (hbdma)
        bytes = 16;

    u8 hdma1 = Retrieve(0xFF51);

    if (hdma1 > 0x7f && hdma1 < 0xa0)
        hdma1 = 0;

    u8 hdma2 = Retrieve(0xFF52);
    u8 hdma3 = Retrieve(0xFF53);
    u8 hdma4 = Retrieve(0xFF54);

    u16 source = (hdma1 << 8) | (hdma2 & 0xF0);
    u16 destination = ((hdma3 & 0x1F) << 8) | (hdma4 & 0xF0);
    destination |= 0x8000;

    if (source >= 0xD000 && source < 0xE000)
    {
        for (int i = 0; i < bytes; i++)
            WriteCGBLCDRAM(destination + i, ReadCGBWRAM(source + i));
    }
    else
    {
        for (int i = 0; i < bytes; i++)
            WriteCGBLCDRAM(destination + i, Read(source + i));
    }
}

bool Memory::IsHBDMAEnabled()
{
    return m_bHBDMAEnabled;
}

