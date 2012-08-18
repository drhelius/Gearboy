/* 
 * Gearboy Gameboy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * The full license is available at http://www.gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "IORegistersMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"

IORegistersMemoryRule::IORegistersMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge) : MemoryRule(pProcessor,
        pMemory, pVideo, pInput, pCartridge)
{
}

u8 IORegistersMemoryRule::PerformRead(u16 address)
{
    if (address == 0xFF00)
    {
        // P1
        return m_pInput->GetJoyPadState();
    }
    else if (address == 0xFF44)
    {
        if (m_pVideo->IsScreenEnabled())
            return m_pMemory->Retrieve(0xFF44);
        else
            return 0x00;
    }
    else
    {
        return m_pMemory->Retrieve(address);
    }
}

void IORegistersMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address == 0xFF00)
    {
        // P1
        m_pMemory->Load(address, value & 0x30);
    }
    else if (address == 0xFF04)
    {
        // DIV
        m_pMemory->Load(address, 0x00);
    }
    else if (address == 0xFF07)
    {
        // TAC
        value &= 0x07;
        u8 current_tac = m_pMemory->Retrieve(0xFF07);
        if ((current_tac & 0x03) != (value & 0x03))
        {
            m_pProcessor->ResetTIMACycles();
        }
        m_pMemory->Load(address, value);
    }
    else if (address == 0xFF0F)
    {
        // IF
        m_pMemory->Load(address, value & 0x1F);
    }
    else if (address == 0xFF40)
    {
        // LCDC
        m_pMemory->Load(address, value);

        if (IsSetBit(value, 7))
            m_pVideo->EnableScreen();
        else
            m_pVideo->DisableScreen();
    }
    else if (address == 0xFF41)
    {
        // STAT
        u8 current_stat_mode = m_pMemory->Retrieve(0xFF41) & 0x03;
        m_pMemory->Load(address, (value & 0x7C) | current_stat_mode);
    }
    else if (address == 0xFF44)
    {
        // LY
        u8 current_ly = m_pMemory->Retrieve(0xFF44);
        if (current_ly & 0x80)
        {
            if ((value & 0x80) == 0)
            {
                m_pVideo->DisableScreen();
            }
        }
    }
    else if (address == 0xFF46)
    {
        // DMA
        m_pMemory->DoDMATransfer(value);
    }
    else if (address == 0xFFFF)
    {
        // IE
        m_pMemory->Load(address, value & 0x1F);
    }
    else
    {
        m_pMemory->Load(address, value);
    }
}

void IORegistersMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}

