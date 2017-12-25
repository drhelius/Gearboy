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

#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearboyCore);
    InitPointer(m_pSoundQueue);
}

Emulator::~Emulator()
{
    SafeDelete(m_pSoundQueue);
    SafeDelete(m_pGearboyCore);
}

void Emulator::Init()
{
    m_pGearboyCore = new GearboyCore();
    m_pGearboyCore->Init();

    m_pSoundQueue = new Sound_Queue();
    m_pSoundQueue->start(44100, 2);
}

void Emulator::LoadRom(const char* szFilePath, bool forceDMG)
{
    m_Mutex.lock();
    m_pGearboyCore->SaveRam();
    m_pGearboyCore->LoadROM(szFilePath, forceDMG);
    m_pGearboyCore->LoadRam();
    m_Mutex.unlock();
}

void Emulator::RunToVBlank(GB_Color* pFrameBuffer)
{
    m_Mutex.lock();

    m_pGearboyCore->RunToVBlank(pFrameBuffer);

    short *audio_buf = NULL;
    long count = 0;
    m_pGearboyCore->GetSamples(&audio_buf, &count);

    if (IsValidPointer(audio_buf) && (count > 0))
        m_pSoundQueue->write(audio_buf, (int)count);

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

void Emulator::Reset(bool forceDMG)
{
    m_Mutex.lock();
    m_pGearboyCore->SaveRam();
    m_pGearboyCore->ResetROM(forceDMG);
    m_pGearboyCore->LoadRam();
    m_Mutex.unlock();
}

void Emulator::MemoryDump()
{
    m_Mutex.lock();
    m_pGearboyCore->GetMemory()->MemoryDump("memdump.txt");
    m_Mutex.unlock();
}

void Emulator::SetSoundSettings(bool enabled, int rate)
{
    m_Mutex.lock();
    //m_pGearboyCore->EnableSound(enabled);
    m_pGearboyCore->SetSoundSampleRate(rate);
    m_Mutex.unlock();
}

void Emulator::SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3,
        GB_Color& color4)
{
    m_Mutex.lock();
    m_pGearboyCore->SetDMGPalette(color1, color2, color3, color4);
    m_Mutex.unlock();
}

void Emulator::SaveRam()
{
    m_Mutex.lock();
    m_pGearboyCore->SaveRam();
    m_Mutex.unlock();
}

bool Emulator::IsCGBRom()
{
    m_Mutex.lock();
    bool cgb = m_pGearboyCore->GetCartridge()->IsCGB();
    m_Mutex.unlock();
    return cgb;
}
