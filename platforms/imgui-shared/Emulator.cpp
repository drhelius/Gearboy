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
    m_bAudioEnabled = true;
    m_bSaveInROMFolder = false;
}

Emulator::~Emulator()
{
    SaveRam();
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

void Emulator::LoadRom(const char* szFilePath, bool forceDMG, bool saveInROMFolder)
{
    //m_Mutex.lock();
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearboyCore->LoadROM(szFilePath, forceDMG);
    LoadRam();
    //m_Mutex.unlock();
}

void Emulator::RunToVBlank(GB_Color* pFrameBuffer)
{
    //m_Mutex.lock();

    s16 sampleBufer[AUDIO_BUFFER_SIZE];
    int sampleCount = 0;

    m_pGearboyCore->RunToVBlank(pFrameBuffer, sampleBufer, &sampleCount);

    if (m_bAudioEnabled && (sampleCount > 0))
    {
        m_pSoundQueue->write(sampleBufer, sampleCount);
    }

    //m_Mutex.unlock();
}

void Emulator::KeyPressed(Gameboy_Keys key)
{
    //m_Mutex.lock();
    m_pGearboyCore->KeyPressed(key);
    //m_Mutex.unlock();
}

void Emulator::KeyReleased(Gameboy_Keys key)
{
    //m_Mutex.lock();
    m_pGearboyCore->KeyReleased(key);
    //m_Mutex.unlock();
}

void Emulator::Pause()
{
    //m_Mutex.lock();
    m_pGearboyCore->Pause(true);
    m_bAudioEnabled = false;
    //m_Mutex.unlock();
}

void Emulator::Resume()
{
    //m_Mutex.lock();
    m_pGearboyCore->Pause(false);
    m_bAudioEnabled = true;
    //m_Mutex.unlock();
}

bool Emulator::IsPaused()
{
    //m_Mutex.lock();
    bool paused = m_pGearboyCore->IsPaused();
    //m_Mutex.unlock();
    return paused;
}

void Emulator::Reset(bool forceDMG, bool saveInROMFolder)
{
    //m_Mutex.lock();
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearboyCore->ResetROM(forceDMG);
    LoadRam();
    //m_Mutex.unlock();
}

void Emulator::MemoryDump()
{
    //m_Mutex.lock();
    m_pGearboyCore->GetMemory()->MemoryDump("memdump.txt");
    //m_Mutex.unlock();
}

void Emulator::SetSoundSettings(bool enabled, int rate)
{
    //m_Mutex.lock();
    m_bAudioEnabled = enabled;
    m_pGearboyCore->SetSoundSampleRate(rate);
    m_pSoundQueue->stop();
    m_pSoundQueue->start(rate, 2);
    //m_Mutex.unlock();
}

void Emulator::SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3,
        GB_Color& color4)
{
    //m_Mutex.lock();
    m_pGearboyCore->SetDMGPalette(color1, color2, color3, color4);
    //m_Mutex.unlock();
}

bool Emulator::IsCGBRom()
{
    //m_Mutex.lock();
    bool cgb = m_pGearboyCore->GetCartridge()->IsCGB();
    //m_Mutex.unlock();
    return cgb;
}

bool Emulator::IsAudioEnabled()
{
    return m_bAudioEnabled;
}

void Emulator::SaveRam()
{
    //if (m_bSaveInROMFolder)
        m_pGearboyCore->SaveRam();
    //else
    //    m_pGearboyCore->SaveRam(QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString().c_str());
}

void Emulator::LoadRam()
{
    //if (m_bSaveInROMFolder)
        m_pGearboyCore->LoadRam();
    //else
    //    m_pGearboyCore->LoadRam(QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString().c_str());
}

void Emulator::SaveState(int index)
{
    //m_Mutex.lock();
    m_pGearboyCore->SaveState(index);
    //m_Mutex.unlock();
}

void Emulator::LoadState(int index)
{
    //m_Mutex.lock();
    m_pGearboyCore->LoadState(index);
    //m_Mutex.unlock();
}
