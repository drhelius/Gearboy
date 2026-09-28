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

#include "MBC5LogoMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

MBC5LogoMemoryRule::MBC5LogoMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5MemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

MBC5LogoMemoryRule::~MBC5LogoMemoryRule()
{
}

bool MBC5LogoMemoryRule::MapsROMDirectly()
{
    return false;
}

void MBC5LogoMemoryRule::Reset(bool bCGB)
{
    MBC5MemoryRule::Reset(bCGB);
    m_LogoMode = (m_pMemory->IsBootromEnabled() && !m_pCartridge->IsBootLogoSwapDisabled()) ? LogoModeDMG : LogoModeDone;
    m_LogoCount = 0;
}

u8 MBC5LogoMemoryRule::PerformRead(u16 address)
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

            if ((m_LogoMode == LogoModeDMG) || ((m_LogoMode == LogoModeUnlocked) && (address >= 0x0104) && (address < 0x0134)))
                address |= 0x80;
        }
    }

    return MBC5MemoryRule::PerformRead(address);
}

bool MBC5LogoMemoryRule::NeedsHighMemoryAccessNotifications()
{
    return true;
}

u8 MBC5LogoMemoryRule::DebugRead(u16 address)
{
    LogoMode mode = m_LogoMode;
    u8 count = m_LogoCount;
    u8 value = PerformRead(address);
    m_LogoMode = mode;
    m_LogoCount = count;
    return value;
}

void MBC5LogoMemoryRule::NotifyHighMemoryWrite(u16 address, u8 value)
{
    UNUSED(value);

    if ((m_LogoMode == LogoModeDMG) && ((address & 0xE000) == 0xC000))
    {
        m_LogoMode = LogoModeCGB;
        m_LogoCount = 0;
    }
}

void MBC5LogoMemoryRule::SaveState(std::ostream& stream)
{
    MBC5MemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_LogoMode), sizeof(m_LogoMode));
    stream.write(reinterpret_cast<const char*>(&m_LogoCount), sizeof(m_LogoCount));
}

void MBC5LogoMemoryRule::LoadState(std::istream& stream)
{
    MBC5MemoryRule::LoadState(stream);
    stream.read(reinterpret_cast<char*>(&m_LogoMode), sizeof(m_LogoMode));
    stream.read(reinterpret_cast<char*>(&m_LogoCount), sizeof(m_LogoCount));
}
