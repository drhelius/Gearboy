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

#include "LiChengMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

LiChengMemoryRule::LiChengMemoryRule(Processor* pProcessor, Memory* pMemory,
        Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5MemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

LiChengMemoryRule::~LiChengMemoryRule()
{
}

bool LiChengMemoryRule::MapsROMDirectly()
{
    return false;
}

u8 LiChengMemoryRule::GetMapperType()
{
    return Cartridge::CartridgeLiCheng;
}

void LiChengMemoryRule::Reset(bool bCGB)
{
    MBC5MemoryRule::Reset(bCGB);
    m_LogoMode = m_pMemory->IsBootromEnabled() ? LogoModeDMG : LogoModeDone;
    m_LogoCount = 0;
}

u8 LiChengMemoryRule::PerformRead(u16 address)
{
    if ((address < 0x8000) && (m_LogoMode != LogoModeDone))
    {
        if ((address == 0x0100) || !m_pMemory->IsBootromRegistryEnabled())
            m_LogoMode = LogoModeDone;
        else
        {
            if (m_LogoCount == 0x30)
            {
                if (m_LogoMode == LogoModeDMG)
                {
                    m_LogoMode = LogoModeCGB;
                    m_LogoCount = 0;
                }
                else if (m_LogoMode == LogoModeCGB)
                    m_LogoMode = LogoModeUnlocked;
            }

            if (m_LogoCount < 0x30)
                m_LogoCount++;

            if ((m_LogoMode == LogoModeDMG) ||
                    ((m_LogoMode == LogoModeUnlocked) && (address >= 0x0104) && (address < 0x0134)))
                address |= 0x80;
        }
    }

    return MBC5MemoryRule::PerformRead(address);
}

void LiChengMemoryRule::PerformWrite(u16 address, u8 value)
{
    // Protection writes must not change the bank. $2100 remains a valid MBC5 register.
    if ((address > 0x2100) && (address < 0x3000))
    {
        TraceMapperEvent(address, value);
        return;
    }

    MBC5MemoryRule::PerformWrite(address, value);
}

bool LiChengMemoryRule::NeedsHighMemoryAccessNotifications()
{
    return true;
}

void LiChengMemoryRule::NotifyHighMemoryWrite(u16 address, u8 value)
{
    UNUSED(value);

    if ((m_LogoMode == LogoModeDMG) && ((address & 0xE000) == 0xC000))
    {
        m_LogoMode = LogoModeCGB;
        m_LogoCount = 0;
    }
}

size_t LiChengMemoryRule::GetRamSize()
{
    return 0x8000;
}

void LiChengMemoryRule::SaveState(std::ostream& stream)
{
    MBC5MemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_LogoMode), sizeof(m_LogoMode));
    stream.write(reinterpret_cast<const char*>(&m_LogoCount), sizeof(m_LogoCount));
}

void LiChengMemoryRule::LoadState(std::istream& stream)
{
    MBC5MemoryRule::LoadState(stream);
    stream.read(reinterpret_cast<char*>(&m_LogoMode), sizeof(m_LogoMode));
    stream.read(reinterpret_cast<char*>(&m_LogoCount), sizeof(m_LogoCount));
}
