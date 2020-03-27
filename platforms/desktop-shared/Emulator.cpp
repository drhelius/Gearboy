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

#include <SDL.h>
#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearboyCore);
    InitPointer(m_pSoundQueue);
    m_bAudioEnabled = true;
    m_bSaveInROMFolder = false;
    InitPointer(m_szDataDir);
    InitPointer(m_pSampleBufer);
}

Emulator::~Emulator()
{
    SaveRam();
    SafeDeleteArray(m_pSampleBufer);
    SafeDelete(m_pSoundQueue);
    SafeDelete(m_pGearboyCore);
    SDL_free(m_szDataDir);
}

void Emulator::Init()
{
    m_pGearboyCore = new GearboyCore();
    m_pGearboyCore->Init();

    m_pSoundQueue = new Sound_Queue();
    m_pSoundQueue->start(44100, 2);

    m_szDataDir = SDL_GetPrefPath("Geardome", GEARBOY_TITLE);

    m_pSampleBufer = new s16[AUDIO_BUFFER_SIZE];

    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        m_pSampleBufer[i] = 0;
}

void Emulator::LoadRom(const char* szFilePath, bool forceDMG, bool saveInROMFolder)
{
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearboyCore->LoadROM(szFilePath, forceDMG);
    LoadRam();
}

void Emulator::RunToVBlank(u16* pFrameBuffer, bool audio_sync)
{
    int sampleCount = 0;

    m_pGearboyCore->RunToVBlank(pFrameBuffer, m_pSampleBufer, &sampleCount);

    if (m_bAudioEnabled && (sampleCount > 0))
    {
        m_pSoundQueue->write(m_pSampleBufer, sampleCount, audio_sync);
    }
}

void Emulator::KeyPressed(Gameboy_Keys key)
{
    m_pGearboyCore->KeyPressed(key);
}

void Emulator::KeyReleased(Gameboy_Keys key)
{
    m_pGearboyCore->KeyReleased(key);
}

void Emulator::Pause()
{
    m_pGearboyCore->Pause(true);
    m_bAudioEnabled = false;
}

void Emulator::Resume()
{
    m_pGearboyCore->Pause(false);
    m_bAudioEnabled = true;
}

bool Emulator::IsPaused()
{
    return m_pGearboyCore->IsPaused();
}

bool Emulator::IsEmpty()
{
    return !m_pGearboyCore->GetCartridge()->IsLoadedROM();
}

void Emulator::Reset(bool forceDMG, bool saveInROMFolder)
{
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearboyCore->ResetROM(forceDMG);
    LoadRam();
}

void Emulator::MemoryDump()
{
    m_pGearboyCore->GetMemory()->MemoryDump("memdump.txt");
}

void Emulator::SetSoundSettings(bool enabled, int rate)
{
    m_bAudioEnabled = enabled;
    m_pGearboyCore->SetSoundSampleRate(rate);
    m_pSoundQueue->stop();
    m_pSoundQueue->start(rate, 2);
}

void Emulator::SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3,
        GB_Color& color4)
{
    m_pGearboyCore->SetDMGPalette(color1, color2, color3, color4);
}

bool Emulator::IsCGBRom()
{
    return m_pGearboyCore->GetCartridge()->IsCGB();
}

bool Emulator::IsAudioEnabled()
{
    return m_bAudioEnabled;
}

void Emulator::SaveRam()
{
    if (m_bSaveInROMFolder)
        m_pGearboyCore->SaveRam();
    else
        m_pGearboyCore->SaveRam(m_szDataDir);
}

void Emulator::LoadRam()
{
    if (m_bSaveInROMFolder)
        m_pGearboyCore->LoadRam();
    else
        m_pGearboyCore->LoadRam(m_szDataDir);
}

void Emulator::SaveState(int index)
{
    m_pGearboyCore->SaveState(index);
}

void Emulator::LoadState(int index)
{
    m_pGearboyCore->LoadState(index);
}
