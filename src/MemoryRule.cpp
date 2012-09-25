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
    m_bCGB = false;
}

MemoryRule::~MemoryRule()
{

}

void MemoryRule::SaveRam(std::ofstream&)
{

}

void MemoryRule::LoadRam(std::ifstream&)
{

}

int MemoryRule::GetRamBanksSize()
{
    return 0;
}

void MemoryRule::AddAddressRange(u16 minAddress, u16 maxAddress)
{
    stAddressRange range;
    range.maxAddress = maxAddress;
    range.minAddress = minAddress;
    m_Ranges.push_back(range);
}

void MemoryRule::ClearAddressRanges()
{
    m_Ranges.clear();
}

bool MemoryRule::IsAddressInRanges(u16 address)
{
    AddressRangeVectorIterator it;

    for (it = m_Ranges.begin(); it < m_Ranges.end(); it++)
    {
        if ((address >= (*it).minAddress) && (address <= (*it).maxAddress))
            return true;
    }

    return false;
}
