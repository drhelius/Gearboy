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

IORegistersMemoryRule::IORegistersMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput, Audio* pAudio)
{
    m_pProcessor = pProcessor;
    m_pMemory = pMemory;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pAudio = pAudio;
    m_bCGB = false;
}

IORegistersMemoryRule::~IORegistersMemoryRule()
{
}

void IORegistersMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}

