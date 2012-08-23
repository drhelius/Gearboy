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

#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearboyCore);
}

Emulator::~Emulator()
{
    SafeDelete(m_pGearboyCore);
}

void Emulator::Init()
{
    m_pGearboyCore = new GearboyCore();
    m_pGearboyCore->Init();
}

void Emulator::LoadRom(const char* szFilePath)
{
    m_Mutex.lock();
    m_pGearboyCore->LoadROM(szFilePath);
    m_Mutex.unlock();
}

void Emulator::RunToVBlank(GB_Color* pFrameBuffer)
{
    m_Mutex.lock();
    m_pGearboyCore->RunToVBlank(pFrameBuffer);
    m_Mutex.unlock();
}

void Emulator::Pause()
{

}

void Emulator::Resume()
{

}
