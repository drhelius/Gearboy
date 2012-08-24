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

#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "IORegistersMemoryRule.h"
#include "RomOnlyMemoryRule.h"
#include "MBC1MemoryRule.h"
#include "MBC2MemoryRule.h"
#include "MBC3MemoryRule.h"
#include "MBC5MemoryRule.h"

GearboyCore::GearboyCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pAudio);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pIORegistersMemoryRule);
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pMBC1MemoryRule);
    InitPointer(m_pMBC2MemoryRule);
    InitPointer(m_pMBC3MemoryRule);
    InitPointer(m_pMBC5MemoryRule);
    m_MBC = MBC_NONE;
    m_bCGB = false;
    m_bPaused = true;
}

GearboyCore::~GearboyCore()
{
    SafeDelete(m_pMBC5MemoryRule);
    SafeDelete(m_pMBC3MemoryRule);
    SafeDelete(m_pMBC2MemoryRule);
    SafeDelete(m_pMBC1MemoryRule);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pIORegistersMemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pAudio);
    SafeDelete(m_pVideo);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearboyCore::Init()
{
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pAudio = new Audio();
    m_pInput = new Input(m_pMemory, m_pProcessor);
    m_pCartridge = new Cartridge();

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pVideo->Init();
    m_pAudio->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    InitMemoryRules();
}

void GearboyCore::RunToVBlank(GB_Color* pFrameBuffer)
{
    if (!m_bPaused && m_pCartridge->IsLoadedROM())
    {
        bool vblank = false;
        while (!vblank)
        {
            u8 clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
        }

        if (!m_bCGB)
            RenderDMGFrame(pFrameBuffer);
    }
}

bool GearboyCore::LoadROM(const char* szFilePath)
{
    bool loaded = m_pCartridge->LoadFromFile(szFilePath);
    if (loaded)
    {
        Reset(m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        bool romTypeOK = AddMemoryRules();

        if (!romTypeOK)
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);

        return romTypeOK;
    }
    else
        return false;
}

Memory* GearboyCore::GetMemory()
{
    return m_pMemory;
}

void GearboyCore::KeyPressed(Gameboy_Keys key)
{
    m_pInput->KeyPressed(key);
}

void GearboyCore::KeyReleased(Gameboy_Keys key)
{
    m_pInput->KeyReleased(key);
}

void GearboyCore::Pause(bool paused)
{
    m_bPaused = paused;
}

bool GearboyCore::IsPaused()
{
    return m_bPaused;
}

void GearboyCore::ResetROM()
{
    if (m_pCartridge->IsLoadedROM())
    {
        Reset(m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        AddMemoryRules();
    }
}

void GearboyCore::InitMemoryRules()
{
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pIORegistersMemoryRule->SetMinAddress(0xFF00);
    m_pIORegistersMemoryRule->SetMaxAddress(0xFFFF);
    m_pIORegistersMemoryRule->Enable();

    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pRomOnlyMemoryRule->Enable();
    m_pRomOnlyMemoryRule->SetMinAddress(0x0000);
    m_pRomOnlyMemoryRule->SetMaxAddress(0xFEFF);

    m_pMBC1MemoryRule = new MBC1MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC1MemoryRule->Enable();
    m_pMBC1MemoryRule->SetMinAddress(0x0000);
    m_pMBC1MemoryRule->SetMaxAddress(0xFEFF);

    m_pMBC2MemoryRule = new MBC2MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC2MemoryRule->Enable();
    m_pMBC2MemoryRule->SetMinAddress(0x0000);
    m_pMBC2MemoryRule->SetMaxAddress(0xFEFF);

    m_pMBC3MemoryRule = new MBC3MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC3MemoryRule->Enable();
    m_pMBC3MemoryRule->SetMinAddress(0x0000);
    m_pMBC3MemoryRule->SetMaxAddress(0xFEFF);

    m_pMBC5MemoryRule = new MBC5MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC5MemoryRule->Enable();
    m_pMBC5MemoryRule->SetMinAddress(0x0000);
    m_pMBC5MemoryRule->SetMaxAddress(0xFEFF);
}

bool GearboyCore::AddMemoryRules()
{
    m_pMemory->AddRule(m_pIORegistersMemoryRule);

    int type = m_pCartridge->GetType();
    if (m_pCartridge->GetROMSize() == 0)
        type = 0;

    bool notSupported = false;

    switch (type)
    {
        case 0x00:
            // NO MBC
            m_pMemory->AddRule(m_pRomOnlyMemoryRule);
            break;
        case 0x01:
            // MBC1
        case 0x02:
            // MBC1
            // SRAM
        case 0x03:
            // MBC1
            // SRAM
            // BATT
            m_pMemory->AddRule(m_pMBC1MemoryRule);
            break;
        case 0x05:
            // MBC2
        case 0x06:
            // MBC2
            // BATT
            m_pMemory->AddRule(m_pMBC2MemoryRule);
            break;
        case 0x08:
            // ROM   
            // SRAM 
        case 0x09:
            // ROM
            // SRAM
            // BATT
            m_pMemory->AddRule(m_pRomOnlyMemoryRule);
            break;
        case 0x0B:
            // MMMO1
        case 0x0C:
            // MMM01   
            // SRAM 
        case 0x0D:
            // MMM01
            // SRAM
            // BATT
            notSupported = true;
            break;
        case 0x0F:
            // MBC3
            // TIMER
            // BATT
        case 0x10:
            // MBC3
            // TIMER
            // BATT
            // SRAM
        case 0x11:
            // MBC3
        case 0x12:
            // MBC3
            // SRAM
        case 0x13:
            // MBC3
            // BATT
            // SRAM
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            break;
        case 0x19:
            // MBC5
        case 0x1A:
            // MBC5
            // SRAM
        case 0x1B:
            // MBC5
            // BATT
            // SRAM
        case 0x1C:
            // RUMBLE
        case 0x1D:
            // RUMBLE
            // SRAM
        case 0x1E:
            // RUMBLE
            // BATT
            // SRAM
            m_pMemory->AddRule(m_pMBC5MemoryRule);
            break;
        case 0x1F:
            // Game Boy Camera
        case 0x22:
            // MBC7
            // BATT
            // SRAM
        case 0xFD:
            // TAMA 5
        case 0xFE:
            // HuC3
        case 0xFF:
            // HuC1
            notSupported = true;
            break;
        default:
            notSupported = true;
            Log("--> ** Unknown cartridge type: %d", type);

            if (notSupported)
                Log("--> ** This cartridge is not supported. Type: %d", type);
    }

    return !notSupported;
}

void GearboyCore::Reset(bool bCGB)
{
    m_bCGB = bCGB;

    if (m_bCGB)
        Log("Switching to Game Boy Color");
    else
        Log("Defaulting to Game Boy DMG");

    m_MBC = MBC_NONE;
    m_pMemory->Reset(m_bCGB);
    m_pProcessor->Reset(m_bCGB);
    m_pVideo->Reset(m_bCGB);
    m_pAudio->Reset();
    m_pInput->Reset();

    m_pIORegistersMemoryRule->Reset(m_bCGB);
    m_pMBC1MemoryRule->Reset(m_bCGB);
    m_pMBC2MemoryRule->Reset(m_bCGB);
    m_pMBC3MemoryRule->Reset(m_bCGB);
    m_pMBC5MemoryRule->Reset(m_bCGB);
    m_pIORegistersMemoryRule->Reset(m_bCGB);

    m_bPaused = false;
}

void GearboyCore::RenderDMGFrame(GB_Color* pFrameBuffer) const
{
    int pixels = GAMEBOY_WIDTH * GAMEBOY_HEIGHT;
    const u8* pGamboyFrameBuffer = m_pVideo->GetFrameBuffer();

    for (int i = 0; i < pixels; i++)
    {
        switch (pGamboyFrameBuffer[i])
        {
            case 0:
                pFrameBuffer[i].red = 0xEF;
                pFrameBuffer[i].green = 0xF3;
                pFrameBuffer[i].blue = 0xD5;
                break;
            case 1:
                pFrameBuffer[i].red = 0xA3;
                pFrameBuffer[i].green = 0xB6;
                pFrameBuffer[i].blue = 0x7A;
                break;
            case 2:
                pFrameBuffer[i].red = 0x37;
                pFrameBuffer[i].green = 0x61;
                pFrameBuffer[i].blue = 0x3B;
                break;
            case 3:
                pFrameBuffer[i].red = 0x04;
                pFrameBuffer[i].green = 0x1C;
                pFrameBuffer[i].blue = 0x16;
                break;
        }
    }
}
