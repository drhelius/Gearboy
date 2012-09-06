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

#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "CommonMemoryRule.h"
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
    InitPointer(m_pCommonMemoryRule);
    InitPointer(m_pIORegistersMemoryRule);
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pMBC1MemoryRule);
    InitPointer(m_pMBC2MemoryRule);
    InitPointer(m_pMBC3MemoryRule);
    InitPointer(m_pMBC5MemoryRule);
    m_MBC = MBC_NONE;
    m_bCGB = false;
    m_bPaused = true;
    m_bForceDMG = false;
}

GearboyCore::~GearboyCore()
{
    SafeDelete(m_pMBC5MemoryRule);
    SafeDelete(m_pMBC3MemoryRule);
    SafeDelete(m_pMBC2MemoryRule);
    SafeDelete(m_pMBC1MemoryRule);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pIORegistersMemoryRule);
    SafeDelete(m_pCommonMemoryRule);
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
    InitDMGPalette();
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
            m_pAudio->Tick(clockCycles);
        }

        if (!m_bCGB)
            RenderDMGFrame(pFrameBuffer);
    }
}

bool GearboyCore::LoadROM(const char* szFilePath, bool forceDMG)
{
    bool loaded = m_pCartridge->LoadFromFile(szFilePath);
    if (loaded)
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        bool romTypeOK = AddMemoryRules();

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);
        }

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

void GearboyCore::ResetROM(bool forceDMG)
{
    if (m_pCartridge->IsLoadedROM())
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        AddMemoryRules();
    }
}

void GearboyCore::EnableSound(bool enabled)
{
    m_pAudio->Enable(enabled);
}

void GearboyCore::SetSoundSampleRate(int rate)
{
    m_pAudio->SetSampleRate(rate);
}

void GearboyCore::SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3,
        GB_Color& color4)
{
    m_DMGPalette[0] = color1;
    m_DMGPalette[1] = color2;
    m_DMGPalette[2] = color3;
    m_DMGPalette[3] = color4;
}

void GearboyCore::InitDMGPalette()
{
    m_DMGPalette[0].red = 0xEF;
    m_DMGPalette[0].green = 0xF3;
    m_DMGPalette[0].blue = 0xD5;
    m_DMGPalette[0].alpha = 0x00;

    m_DMGPalette[1].red = 0xA3;
    m_DMGPalette[1].green = 0xB6;
    m_DMGPalette[1].blue = 0x7A;
    m_DMGPalette[1].alpha = 0x00;

    m_DMGPalette[2].red = 0x37;
    m_DMGPalette[2].green = 0x61;
    m_DMGPalette[2].blue = 0x3B;
    m_DMGPalette[2].alpha = 0x00;

    m_DMGPalette[3].red = 0x04;
    m_DMGPalette[3].green = 0x1C;
    m_DMGPalette[3].blue = 0x16;
    m_DMGPalette[3].alpha = 0x00;
}

void GearboyCore::InitMemoryRules()
{
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pIORegistersMemoryRule->AddAddressRange(0xFF00, 0xFFFF);

    m_pCommonMemoryRule = new CommonMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pCommonMemoryRule->AddAddressRange(0x8000, 0x9FFF);
    m_pCommonMemoryRule->AddAddressRange(0xC000, 0xFEFF);

    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pRomOnlyMemoryRule->AddAddressRange(0x0000, 0x7FFF);
    m_pRomOnlyMemoryRule->AddAddressRange(0xA000, 0xBFFF);

    m_pMBC1MemoryRule = new MBC1MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pMBC1MemoryRule->AddAddressRange(0x0000, 0x7FFF);
    m_pMBC1MemoryRule->AddAddressRange(0xA000, 0xBFFF);

    m_pMBC2MemoryRule = new MBC2MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pMBC2MemoryRule->AddAddressRange(0x0000, 0x7FFF);
    m_pMBC2MemoryRule->AddAddressRange(0xA000, 0xBFFF);

    m_pMBC3MemoryRule = new MBC3MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pMBC3MemoryRule->AddAddressRange(0x0000, 0x7FFF);
    m_pMBC3MemoryRule->AddAddressRange(0xA000, 0xBFFF);

    m_pMBC5MemoryRule = new MBC5MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
    m_pMBC5MemoryRule->AddAddressRange(0x0000, 0x7FFF);
    m_pMBC5MemoryRule->AddAddressRange(0xA000, 0xBFFF);
}

bool GearboyCore::AddMemoryRules()
{
    m_pMemory->AddRule(m_pIORegistersMemoryRule);
    m_pMemory->AddRule(m_pCommonMemoryRule);

    int type = m_pCartridge->GetType();
    if ((type != 0xEA) && (m_pCartridge->GetROMSize() == 0))
        type = 0;

    bool notSupported = false;

    switch (type)
    {
        case 0x00:
            // NO MBC
        case 0x08:
            // ROM   
            // SRAM 
        case 0x09:
            // ROM
            // SRAM
            // BATT
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
        case 0xEA:
            // Hack to accept 0xEA as a MBC1 memory bank controller (Sonic 3D Blast 5)
            m_pMemory->AddRule(m_pMBC1MemoryRule);
            break;
        case 0x05:
            // MBC2
        case 0x06:
            // MBC2
            // BATT
            m_pMemory->AddRule(m_pMBC2MemoryRule);
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
        case 0xFC:
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
        case 0x0B:
            // MMMO1
        case 0x0C:
            // MMM01   
            // SRAM 
        case 0x0D:
            // MMM01
            // SRAM
            // BATT
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
            Log("--> ** This cartridge is not supported. Type: %d", type);
            break;
        default:
            notSupported = true;
            Log("--> ** Unknown cartridge type: %d", type);
    }

    return !notSupported;
}

void GearboyCore::Reset(bool bCGB)
{
    m_bCGB = bCGB;

    if (m_bCGB)
    {
        Log("Switching to Game Boy Color");
    }
    else
    {
        Log("Defaulting to Game Boy DMG");
    }

    m_MBC = MBC_NONE;
    m_pMemory->Reset(m_bCGB);
    m_pProcessor->Reset(m_bCGB);
    m_pVideo->Reset(m_bCGB);
    m_pAudio->Reset(m_bCGB);
    m_pInput->Reset();

    m_pCommonMemoryRule->Reset(m_bCGB);
    m_pRomOnlyMemoryRule->Reset(m_bCGB);
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
    const u8* pGameboyFrameBuffer = m_pVideo->GetFrameBuffer();

    for (int i = 0; i < pixels; i++)
    {
        pFrameBuffer[i] = m_DMGPalette[pGameboyFrameBuffer[i]];
    }
}
