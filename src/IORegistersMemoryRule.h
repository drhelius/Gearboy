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

#ifndef IOREGISTERSMEMORYRULE_H
#define	IOREGISTERSMEMORYRULE_H

#include "definitions.h"

class Video;
class Processor;
class Input;
class Audio;
class Memory;

class IORegistersMemoryRule
{
public:
    IORegistersMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Audio* pAudio);
    ~IORegistersMemoryRule();
    u8 PerformRead(u16 address);
    void PerformWrite(u16 address, u8 value);
    void Reset(bool bCGB);
    
private:
    Processor* m_pProcessor;
    Memory* m_pMemory;
    Video* m_pVideo;
    Input* m_pInput;
    Audio* m_pAudio;
    bool m_bCGB;
};

#include "Video.h"
#include "Processor.h"
#include "Input.h"
#include "Audio.h"
#include "Memory.h"

inline u8 IORegistersMemoryRule::PerformRead(u16 address)
{
    switch (address)
    {
        case 0xFF00:
        {
            // P1
            return m_pInput->Read();
        }
        case 0xFF03:
        {
            // UNDOCUMENTED
            return 0xFF;
        }
        case 0xFF07:
        {
            // TAC
            return m_pMemory->Retrieve(0xFF07) | 0xF8;
        }
        case 0xFF08:
        case 0xFF09:
        case 0xFF0A:
        case 0xFF0B:
        case 0xFF0C:
        case 0xFF0D:
        case 0xFF0E:
        {
            // UNDOCUMENTED
            return 0xFF;
        }
        case 0xFF0F:
        {
            // IF
            return m_pMemory->Retrieve(0xFF0F) | 0xE0;
        }
        case 0xFF10:
        case 0xFF11:
        case 0xFF12:
        case 0xFF13:
        case 0xFF14:
        case 0xFF15:
        case 0xFF16:
        case 0xFF17:
        case 0xFF18:
        case 0xFF19:
        case 0xFF1A:
        case 0xFF1B:
        case 0xFF1C:
        case 0xFF1D:
        case 0xFF1E:
        case 0xFF1F:
        case 0xFF20:
        case 0xFF21:
        case 0xFF22:
        case 0xFF23:
        case 0xFF24:
        case 0xFF25:
        case 0xFF26:
        case 0xFF27:
        case 0xFF28:
        case 0xFF29:
        case 0xFF2A:
        case 0xFF2B:
        case 0xFF2C:
        case 0xFF2D:
        case 0xFF2E:
        case 0xFF2F:
        case 0xFF30:
        case 0xFF31:
        case 0xFF32:
        case 0xFF33:
        case 0xFF34:
        case 0xFF35:
        case 0xFF36:
        case 0xFF37:
        case 0xFF38:
        case 0xFF39:
        case 0xFF3A:
        case 0xFF3B:
        case 0xFF3C:
        case 0xFF3D:
        case 0xFF3E:
        case 0xFF3F:
        {
            // SOUND REGISTERS
            return m_pAudio->ReadAudioRegister(address);
        }
        case 0xFF41:
        {
            // STAT
            return m_pMemory->Retrieve(0xFF41) | 0x80;
        }
        case 0xFF44:
        {
            return (m_pVideo->IsScreenEnabled() ? m_pMemory->Retrieve(0xFF44) : 0x00);
        }
        case 0xFF4C:
        {
            // UNDOCUMENTED
            return 0xFF;
        }
        case 0xFF4F:
        {
            // VBK
            return m_pMemory->Retrieve(0xFF4F) | 0xFE;
        }
        case 0xFF51:
        {
            // HDMA1
            return (m_bCGB ? m_pMemory->GetHDMARegister(1) : m_pMemory->Retrieve(address));
        }
        case 0xFF52:
        {
            // HDMA2
            return (m_bCGB ? m_pMemory->GetHDMARegister(2) : m_pMemory->Retrieve(address));
        }
        case 0xFF53:
        {
            // HDMA3
            return (m_bCGB ? m_pMemory->GetHDMARegister(3) : m_pMemory->Retrieve(address));
        }
        case 0xFF54:
        {
            // HDMA4
            return (m_bCGB ? m_pMemory->GetHDMARegister(4) : m_pMemory->Retrieve(address));
        }
        case 0xFF55:
        {
            // DMA CGB
            return (m_bCGB ? m_pMemory->GetHDMARegister(5) : m_pMemory->Retrieve(address));
        }
        case 0xFF68:
        case 0xFF6A:
        {
            // BCPS, OCPS
            return (m_bCGB ? (m_pMemory->Retrieve(address) | 0x40) : 0xC0);
        }
        case 0xFF69:
        case 0xFF6B:
        {
            // BCPD, OCPD
            return (m_bCGB ? m_pMemory->Retrieve(address) : 0xFF);
        }
        case 0xFF70:
        {
            // SVBK
            return (m_bCGB ? (m_pMemory->Retrieve(0xFF70) | 0xF8) : 0xFF);
        }
        case 0xFF76:
        {
            // UNDOCUMENTED
            return (m_bCGB ? 0x00 : 0xFF);
        }
        case 0xFF77:
        {
            // UNDOCUMENTED
            return (m_bCGB ? 0x0 : 0xFF);
        }
    }

    return m_pMemory->Retrieve(address);
}

inline void IORegistersMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address)
    {
        case 0xFF00:
        {
            // P1
            m_pInput->Write(value);
            break;
        }
        case 0xFF04:
        {
            // DIV
            m_pProcessor->ResetDIVCycles();
            break;
        }
        case 0xFF07:
        {
            // TAC
            value &= 0x07;
            u8 current_tac = m_pMemory->Retrieve(0xFF07);
            if ((current_tac & 0x03) != (value & 0x03))
            {
                m_pProcessor->ResetTIMACycles();
            }
            m_pMemory->Load(address, value);
            break;
        }
        case 0xFF0F:
        {
            // IF
            m_pMemory->Load(address, value & 0x1F);
            break;
        }
        case 0xFF10:
        case 0xFF11:
        case 0xFF12:
        case 0xFF13:
        case 0xFF14:
        case 0xFF15:
        case 0xFF16:
        case 0xFF17:
        case 0xFF18:
        case 0xFF19:
        case 0xFF1A:
        case 0xFF1B:
        case 0xFF1C:
        case 0xFF1D:
        case 0xFF1E:
        case 0xFF1F:
        case 0xFF20:
        case 0xFF21:
        case 0xFF22:
        case 0xFF23:
        case 0xFF24:
        case 0xFF25:
        case 0xFF26:
        case 0xFF27:
        case 0xFF28:
        case 0xFF29:
        case 0xFF2A:
        case 0xFF2B:
        case 0xFF2C:
        case 0xFF2D:
        case 0xFF2E:
        case 0xFF2F:
        case 0xFF30:
        case 0xFF31:
        case 0xFF32:
        case 0xFF33:
        case 0xFF34:
        case 0xFF35:
        case 0xFF36:
        case 0xFF37:
        case 0xFF38:
        case 0xFF39:
        case 0xFF3A:
        case 0xFF3B:
        case 0xFF3C:
        case 0xFF3D:
        case 0xFF3E:
        case 0xFF3F:
        {
            // SOUND REGISTERS
            m_pAudio->WriteAudioRegister(address, value);
            break;
        }
        case 0xFF40:
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
            break;
        }
        case 0xFF41:
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
            break;
        }
        case 0xFF44:
        {
            // LY
            u8 current_ly = m_pMemory->Retrieve(0xFF44);
            if (IsSetBit(current_ly, 7) && !IsSetBit(value, 7))
            {
                m_pVideo->DisableScreen();
            }
            break;
        }
        case 0xFF45:
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
            break;
        }
        case 0xFF46:
        {
            // DMA
            m_pMemory->Load(address, value);
            m_pMemory->PerformDMA(value);
            break;
        }
        case 0xFF4D:
        {
            // KEY1
            if (m_bCGB)
            {
                u8 current_key1 = m_pMemory->Retrieve(0xFF4D);
                m_pMemory->Load(address, (current_key1 & 0x80) | (value & 0x01) | 0x7E);
            }
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF4F:
        {
            // VBK
            if (m_bCGB)
            {
                value &= 0x01;
                m_pMemory->SwitchCGBLCDRAM(value);
            }
            m_pMemory->Load(address, value);
            break;
        }
        case 0xFF51:
        {
            // HDMA1
            if (m_bCGB)
                m_pMemory->SetHDMARegister(1, value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF52:
        {
            // HDMA2
            if (m_bCGB)
                m_pMemory->SetHDMARegister(2, value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF53:
        {
            // HDMA3
            if (m_bCGB)
                m_pMemory->SetHDMARegister(3, value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF54:
        {
            // HDMA4
            if (m_bCGB)
                m_pMemory->SetHDMARegister(4, value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF55:
        {
            // DMA CGB
            if (m_bCGB)
                m_pMemory->SwitchCGBDMA(value);
            else
                m_pMemory->Load(address, value);
            break;
        }
        case 0xFF68:
        {
            // BCPS
            m_pMemory->Load(address, value);
            if (m_bCGB)
                m_pVideo->UpdatePaletteToSpecification(true, value);
            break;
        }
        case 0xFF69:
        {
            // BCPD
            m_pMemory->Load(address, value);
            if (m_bCGB)
                m_pVideo->SetColorPalette(true, value);
            break;
        }
        case 0xFF6A:
        {
            // OCPS
            m_pMemory->Load(address, value);
            if (m_bCGB)
                m_pVideo->UpdatePaletteToSpecification(false, value);
            break;
        }
        case 0xFF6B:
        {
            // OCPD
            m_pMemory->Load(address, value);
            if (m_bCGB)
                m_pVideo->SetColorPalette(false, value);
            break;
        }
        case 0xFF6C:
        {
            // UNDOCUMENTED
            m_pMemory->Load(0xFF6C, value | 0xFE);
            break;
        }
        case 0xFF70:
        {
            // SVBK
            if (m_bCGB)
            {
                value &= 0x07;
                m_pMemory->SwitchCGBWRAM(value);
            }
            m_pMemory->Load(address, value);
            break;
        }
        case 0xFF75:
        {
            // UNDOCUMENTED
            m_pMemory->Load(0xFF75, value | 0x8F);
            break;
        }
        case 0xFFFF:
        {
            // IE
            m_pMemory->Load(address, value & 0x1F);
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
        }
    }
}

#endif	/* IOREGISTERSMEMORYRULE_H */

