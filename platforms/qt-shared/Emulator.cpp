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

void Emulator::KeyPressed(Gameboy_Keys key)
{
    m_Mutex.lock();
    m_pGearboyCore->KeyPressed(key);
    m_Mutex.unlock();
}

void Emulator::KeyReleased(Gameboy_Keys key)
{
    m_Mutex.lock();
    m_pGearboyCore->KeyReleased(key);
    m_Mutex.unlock();
}

void Emulator::Pause()
{
    m_Mutex.lock();
    m_pGearboyCore->Pause(true);
    m_Mutex.unlock();
}

void Emulator::Resume()
{
    m_Mutex.lock();
    m_pGearboyCore->Pause(false);
    m_Mutex.unlock();
}

bool Emulator::IsPaused()
{
    m_Mutex.lock();
    bool paused = m_pGearboyCore->IsPaused();
    m_Mutex.unlock();
    return paused;
}

void Emulator::Reset()
{
    m_Mutex.lock();
    m_pGearboyCore->ResetROM();
    m_Mutex.unlock();
}
