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
    InitPointer(m_pRamChangedCallback);
}

MemoryRule::~MemoryRule()
{

}

void MemoryRule::SaveRam(std::ostream&)
{
    Log("MemoryRule::SaveRam not implemented");
}

bool MemoryRule::LoadRam(std::istream&, s32)
{
    Log("MemoryRule::LoadRam not implemented");
    return false;
}

void MemoryRule::SetRamChangedCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

size_t MemoryRule::GetRamSize()
{
    Log("MemoryRule::GetRamSize not implemented");
    return 0;
}

size_t MemoryRule::GetRTCSize()
{
    Log("MemoryRule::GetRTCSize not implemented");
    return 0;
}

u8* MemoryRule::GetRamBanks()
{
    Log("MemoryRule::GetRamBanks not implemented");
    return NULL;
}

u8* MemoryRule::GetCurrentRamBank()
{
    Log("MemoryRule::GetCurrentRamBank not implemented");
    return NULL;
}

u8* MemoryRule::GetRomBank0()
{
    Log("MemoryRule::GetRomBank0 not implemented");
    return NULL;
}

u8* MemoryRule::GetCurrentRomBank1()
{
    Log("MemoryRule::GetCurrentRomBank1 not implemented");
    return NULL;
}

u8* MemoryRule::GetRTCMemory()
{
    Log("MemoryRule::GetRTCMemory not implemented");
    return NULL;
}

void MemoryRule::SaveState(std::stringstream& stream)
{
    Log("MemoryRule::SaveState not implemented");
}

void MemoryRule::LoadState(std::stringstream& stream)
{
    Log("MemoryRule::LoadState not implemented");
}
