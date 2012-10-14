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

#include "IORegistersMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Audio.h"

IORegistersMemoryRule::IORegistersMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
}

IORegistersMemoryRule::~IORegistersMemoryRule()
{
}

u8 IORegistersMemoryRule::PerformRead(u16 address)
{
    if (address == 0xFF00)
    {
        // P1
        return m_pInput->Read();
    }
    else if (address == 0xFF03)
    {
        // UNDOCUMENTED
        return 0xFF;
    }
    else if (address == 0xFF07)
    {
        // TAC
        return m_pMemory->Retrieve(0xFF07) | 0xF8;
    }
    else if ((address >= 0xFF08) && (address <= 0xFF0E))
    {
        // UNDOCUMENTED
        return 0xFF;
    }
    else if (address == 0xFF0F)
    {
        // IF
        return m_pMemory->Retrieve(0xFF0F) | 0xE0;
    }
    else if ((address >= 0xFF10) && (address <= 0xFF3F))
    {
        // SOUND REGISTERS
        return m_pAudio->ReadAudioRegister(address);
    }
    else if (address == 0xFF41)
    {
        // STAT
        return m_pMemory->Retrieve(0xFF41) | 0x80;
    }
    else if (address == 0xFF44)
    {
        if (m_pVideo->IsScreenEnabled())
            return m_pMemory->Retrieve(0xFF44);
        else
            return 0x00;
    }
    else if (address == 0xFF4C)
    {
        // UNDOCUMENTED
        return 0xFF;
    }
    else if (address == 0xFF4F)
    {
        // VBK
        return m_pMemory->Retrieve(0xFF4F) | 0xFE;
    }
    else if (m_bCGB && (address == 0xFF51))
    {
        // HDMA1
        return m_pMemory->GetHDMARegister(1);
    }
    else if (m_bCGB && (address == 0xFF52))
    {
        // HDMA2
        return m_pMemory->GetHDMARegister(2);
    }
    else if (m_bCGB && (address == 0xFF53))
    {
        // HDMA3
        return m_pMemory->GetHDMARegister(3);
    }
    else if (m_bCGB && (address == 0xFF54))
    {
        // HDMA4
        return m_pMemory->GetHDMARegister(4);
    }
    else if (m_bCGB && (address == 0xFF55))
    {
        // DMA CGB
        return m_pMemory->GetHDMARegister(5);
    }
    else if ((address == 0xFF68) || (address == 0xFF6A))
    {
        // BCPS, OCPS
        return (m_bCGB ? (m_pMemory->Retrieve(address) | 0x40) : 0xC0);
    }
    else if ((address == 0xFF69) || (address == 0xFF6B))
    {
        // BCPD, OCPD
        return (m_bCGB ? m_pMemory->Retrieve(address) : 0xFF);
    }
    else if (address == 0xFF70)
    {
        // SVBK
        return (m_bCGB ? (m_pMemory->Retrieve(0xFF70) | 0xF8) : 0xFF);
    }
    else if (address == 0xFF76)
    {
        // UNDOCUMENTED
        return (m_bCGB ? 0x00 : 0xFF);
    }
    else if (address == 0xFF77)
    {
        // UNDOCUMENTED
        return (m_bCGB ? 0x0 : 0xFF);
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
        m_pInput->Write(value);
    }
    else if (address == 0xFF04)
    {
        // DIV
        m_pProcessor->ResetDIVCycles();
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
    else if ((address >= 0xFF10) && (address <= 0xFF3F))
    {
        // SOUND REGISTERS
        m_pAudio->WriteAudioRegister(address, value);
    }
    else if (address == 0xFF40)
    {
        // LCDC
        u8 current_lcdc = m_pMemory->Retrieve(0xFF40);
        u8 new_lcdc = value;
        m_pMemory->Load(address, new_lcdc);

        if (!IsSetBit(current_lcdc, 5) && IsSetBit(new_lcdc, 5))
            m_pVideo->ResetWindowLine();

        if (IsSetBit(new_lcdc, 7))
            m_pVideo->EnableScreen();
        else
            m_pVideo->DisableScreen();
    }
    else if (address == 0xFF41)
    {
        // STAT
        u8 current_stat = m_pMemory->Retrieve(0xFF41) & 0x07;
        u8 new_stat = (value & 0x78) | (current_stat & 0x07);
        m_pMemory->Load(address, new_stat);

        u8 lcdc = m_pMemory->Retrieve(0xFF40);
        u8 signal = m_pVideo->GetIRQ48Signal();
        int mode = m_pVideo->GetCurrentStatusMode();

        signal &= ((new_stat >> 3) & 0x0F);
        m_pVideo->SetIRQ48Signal(signal);

        if (IsSetBit(lcdc, 7))
        {
            if (IsSetBit(new_stat, 3) && (mode == 0))
            {
                if (signal == 0)
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                }
                signal = SetBit(signal, 0);
            }

            if (IsSetBit(new_stat, 4) && (mode == 1))
            {
                if (signal == 0)
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                }
                signal = SetBit(signal, 1);
            }

            if (IsSetBit(new_stat, 5) && (mode == 2))
            {
                if (signal == 0)
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                }
                signal = SetBit(signal, 2);
            }

            m_pVideo->CompareLYToLYC();
        }
    }
    else if (address == 0xFF44)
    {
        // LY
        u8 current_ly = m_pMemory->Retrieve(0xFF44);
        if (IsSetBit(current_ly, 7) && !IsSetBit(value, 7))
        {
            m_pVideo->DisableScreen();
        }
    }
    else if (address == 0xFF45)
    {
        // LYC
        u8 current_lyc = m_pMemory->Retrieve(0xFF45);
        if (current_lyc != value)
        {
            m_pMemory->Load(0xFF45, value);
            u8 lcdc = m_pMemory->Retrieve(0xFF40);
            if (IsSetBit(lcdc, 7))
            {
                m_pVideo->CompareLYToLYC();
            }
        }
    }
    else if (address == 0xFF46)
    {
        // DMA
        m_pMemory->Load(address, value);
        m_pMemory->PerformDMA(value);
    }
    else if (m_bCGB && (address == 0xFF4D))
    {
        // KEY1
        u8 current_key1 = m_pMemory->Retrieve(0xFF4D);
        m_pMemory->Load(address, (current_key1 & 0x80) | (value & 0x01) | 0x7E);
    }
    else if (m_bCGB && (address == 0xFF4F))
    {
        // VBK
        value &= 0x01;
        m_pMemory->SwitchCGBLCDRAM(value);
        m_pMemory->Load(address, value);
    }
    else if (m_bCGB && (address == 0xFF51))
    {
        // HDMA1
        m_pMemory->SetHDMARegister(1, value);
    }
    else if (m_bCGB && (address == 0xFF52))
    {
        // HDMA2
        m_pMemory->SetHDMARegister(2, value);
    }
    else if (m_bCGB && (address == 0xFF53))
    {
        // HDMA3
        m_pMemory->SetHDMARegister(3, value);
    }
    else if (m_bCGB && (address == 0xFF54))
    {
        // HDMA4
        m_pMemory->SetHDMARegister(4, value);
    }
    else if (m_bCGB && (address == 0xFF55))
    {
        // DMA CGB
        m_pMemory->SwitchCGBDMA(value);
    }
    else if (m_bCGB && (address == 0xFF69))
    {
        // BCPD
        m_pVideo->SetColorPalette(true, value);
        m_pMemory->Load(address, value);
    }
    else if (m_bCGB && (address == 0xFF6B))
    {
        // OCPD
        m_pVideo->SetColorPalette(false, value);
        m_pMemory->Load(address, value);
    }
    else if (address == 0xFF6C)
    {
        // UNDOCUMENTED
        m_pMemory->Load(0xFF6C, value | 0xFE);
    }
    else if (m_bCGB && (address == 0xFF70))
    {
        // SVBK
        value &= 0x07;
        m_pMemory->SwitchCGBWRAM(value);
        m_pMemory->Load(address, value);
    }
    else if (address == 0xFF75)
    {
        // UNDOCUMENTED
        m_pMemory->Load(0xFF75, value | 0x8F);
    }
    else if (address == 0xFFFF)
    {
        // IE
        m_pMemory->Load(address, value & 0x1F);
    }
    else
        m_pMemory->Load(address, value);
}

void IORegistersMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}

