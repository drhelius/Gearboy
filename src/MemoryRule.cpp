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

#include "MemoryRule.h"

MemoryRule::MemoryRule(Processor* pProcessor, Memory* pMemory,
        Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio)
{
    m_pProcessor = pProcessor;
    m_pMemory = pMemory;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pAudio = pAudio;
    m_bEnabled = false;
    m_MaxAddress = 0;
    m_bCGB = false;
}

MemoryRule::~MemoryRule()
{
    
}

void MemoryRule::SetMaxAddress(u16 maxAddress)
{
    this->m_MaxAddress = maxAddress;
}

u16 MemoryRule::GetMaxAddress() const
{
    return m_MaxAddress;
}

void MemoryRule::SetMinAddress(u16 minAddress)
{
    this->m_MinAddress = minAddress;
}

u16 MemoryRule::GetMinAddress() const
{
    return m_MinAddress;
}

bool MemoryRule::IsEnabled() const
{
    return m_bEnabled;
}

void MemoryRule::Enable()
{
    m_bEnabled = true;
}

void MemoryRule::Disable()
{
    m_bEnabled = false;
}

bool MemoryRule::IsAddressInRange(u16 address)
{
    return (address >= m_MinAddress) && (address <= m_MaxAddress);
}
