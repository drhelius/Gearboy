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
 * along with this program.  If not, see https://www.gnu.org/licenses/ 
 * 
 */

#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "MemoryRule.h"
#include "CommonMemoryRule.h"
#include "IORegistersMemoryRule.h"
#include "RomOnlyMemoryRule.h"
#include "MBC1MemoryRule.h"
#include "MBC2MemoryRule.h"
#include "MBC3MemoryRule.h"
#include "MBC5MemoryRule.h"
#include "MultiMBC1MemoryRule.h"

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
    InitPointer(m_pMultiMBC1MemoryRule);
    InitPointer(m_pMBC2MemoryRule);
    InitPointer(m_pMBC3MemoryRule);
    InitPointer(m_pMBC5MemoryRule);
    m_bCGB = false;
    m_bPaused = true;
    m_bForceDMG = false;
    m_bRTCUpdateCount = 0;
    m_bDuringBootROM = false;
    m_bLoadRamPending = false;
    m_szLoadRamPendingPath[0] = 0;
    InitPointer(m_pRamChangedCallback);
}

GearboyCore::~GearboyCore()
{
#ifdef DEBUG_GEARBOY
    if (m_pCartridge->IsLoadedROM())
    {
        Log("Saving Memory Dump...");

        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dump");

        m_pMemory->MemoryDump(path);

        Log("Memory Dump Saved");
    }
#endif

    SafeDelete(m_pMBC5MemoryRule);
    SafeDelete(m_pMBC3MemoryRule);
    SafeDelete(m_pMBC2MemoryRule);
    SafeDelete(m_pMultiMBC1MemoryRule);
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
            unsigned int clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
            m_pAudio->Tick(clockCycles);
            m_pInput->Tick(clockCycles);

            if (m_bDuringBootROM && m_pProcessor->BootROMfinished())
            {
                m_bDuringBootROM = false;
                Reset(m_bCGB);
                m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
                AddMemoryRules();
                if (m_bLoadRamPending)
                {
                    m_bLoadRamPending = false;
                    LoadRam((m_szLoadRamPendingPath[0] == 0) ? NULL : m_szLoadRamPendingPath);
                }
                break;
            }
        }

        m_bRTCUpdateCount++;
        if (m_bRTCUpdateCount == 50)
        {
            m_bRTCUpdateCount = 0;
            m_pCartridge->UpdateCurrentRTC();
        }

        if (!m_bCGB && IsValidPointer(pFrameBuffer))
            RenderDMGFrame(pFrameBuffer);
    }
}

bool GearboyCore::LoadROM(const char* szFilePath, bool forceDMG)
{
#ifdef DEBUG_GEARBOY
    if (m_pCartridge->IsLoadedROM())
    {
        Log("Saving Memory Dump...");

        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dump");

        m_pMemory->MemoryDump(path);

        Log("Memory Dump Saved");
    }
#endif

    bool loaded = m_pCartridge->LoadFromFile(szFilePath);
    if (loaded)
    {
        m_bDuringBootROM = true;
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

Cartridge* GearboyCore::GetCartridge()
{
    return m_pCartridge;
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
        m_bDuringBootROM = true;
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

void GearboyCore::ResetSound(bool soft)
{
    m_pAudio->Reset(m_bCGB, soft);
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
    m_DMGPalette[0].alpha = 0xFF;
    m_DMGPalette[1].alpha = 0xFF;
    m_DMGPalette[2].alpha = 0xFF;
    m_DMGPalette[3].alpha = 0xFF;
}

void GearboyCore::SaveRam()
{
    SaveRam(NULL);
}

void GearboyCore::SaveRam(const char* szPath)
{
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Saving RAM...");

        using namespace std;

        char path[512];

        if (IsValidPointer(szPath))
        {
            strcpy(path, szPath);
            strcat(path, "/");
            strcat(path, m_pCartridge->GetFileName());
        }
        else
        {
            strcpy(path, m_pCartridge->GetFilePath());
        }

        strcat(path, ".gearboy");

        Log("Save file: %s", path);

        ofstream file(path, ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Log("RAM saved");
    }
}

void GearboyCore::LoadRam()
{
    LoadRam(NULL);
}

bool GearboyCore::LoadSaveFile(std::ifstream& file)
{
    if (file.fail() || file.eof())
    {
        return false;
    }

    char signature[16];

    file.read(signature, 16);
    if (!file)
    {
        return false;
    }

    if (strcmp(signature, SAVE_FILE_SIGNATURE) == 0)
    {
        Log("Old save format: loading header...");

        u8 version;
        char romName[16];
        u8 romType;
        u8 romSize;
        u8 ramSize;
        u8 ramBanksSize;
        u8 ramBanksStart;
        u8 saveStateSize;
        u8 saveStateStart;

        file.read(reinterpret_cast<char*> (&version), 1);
        file.read(romName, 16);
        file.read(reinterpret_cast<char*> (&romType), 1);
        file.read(reinterpret_cast<char*> (&romSize), 1);
        file.read(reinterpret_cast<char*> (&ramSize), 1);
        file.read(reinterpret_cast<char*> (&ramBanksSize), 1);
        file.read(reinterpret_cast<char*> (&ramBanksStart), 1);
        file.read(reinterpret_cast<char*> (&saveStateSize), 1);
        file.read(reinterpret_cast<char*> (&saveStateStart), 1);

        Log("Header loaded");

        m_pMemory->GetCurrentRule()->LoadRam(file, 0);

        Log("RAM loaded");
        return true;
    }

    file.seekg(0, file.end);
    s32 fileSize = (s32)file.tellg();
    file.seekg(0, file.beg);

    if (m_pMemory->GetCurrentRule()->LoadRam(file, fileSize))
    {
        Log("RAM loaded");
        return true;
    }
    Log("Save file size incorrect: %d", fileSize);
    return false;
}

bool GearboyCore::LoadSaveFileFromPath(const char* path)
{
    Log("Opening save file: %s", path);
    std::ifstream file(path, std::ios::in | std::ios::binary);
    return LoadSaveFile(file);
}

bool GearboyCore::LoadSave(const char* szPath)
{
    char path[512];

    if (IsValidPointer(szPath))
    {
        strcpy(path, szPath);
        strcat(path, "/");
        strcat(path, m_pCartridge->GetFileName());
    }
    else
    {
        strcpy(path, m_pCartridge->GetFilePath());
    }

    strcat(path, ".gearboy");
    if(LoadSaveFileFromPath(path))
    {
        return true;
    }

    char* ptr;
    ptr = strrchr(path, '.');
    if (ptr == NULL)
    {
        return false;
    }
    *ptr = '\0';
    ptr = strrchr(path, '.');
    if (ptr == NULL)
    {
        return false;
    }
    *(++ptr) = 's';
    *(++ptr) = 'a';
    *(++ptr) = 'v';
    *(++ptr) = '\0';
    return LoadSaveFileFromPath(path);
}

void GearboyCore::LoadRam(const char* szPath)
{
    if (m_bDuringBootROM)
    {
        m_bLoadRamPending = true;
        if (IsValidPointer(szPath))
            strcpy(m_szLoadRamPendingPath, szPath);
        else 
            m_szLoadRamPendingPath[0] = 0;
        return;
    }
    
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Loading RAM...");

        if(!LoadSave(szPath))
        {
            Log("Save file doesn't exist");
        }
    }
}

void GearboyCore::SetRamModificationCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

void GearboyCore::InitDMGPalette()
{
    m_DMGPalette[0].red = 0x87;
    m_DMGPalette[0].green = 0x96;
    m_DMGPalette[0].blue = 0x03;
    m_DMGPalette[0].alpha = 0xFF;

    m_DMGPalette[1].red = 0x4d;
    m_DMGPalette[1].green = 0x6b;
    m_DMGPalette[1].blue = 0x03;
    m_DMGPalette[1].alpha = 0xFF;

    m_DMGPalette[2].red = 0x2b;
    m_DMGPalette[2].green = 0x55;
    m_DMGPalette[2].blue = 0x03;
    m_DMGPalette[2].alpha = 0xFF;

    m_DMGPalette[3].red = 0x14;
    m_DMGPalette[3].green = 0x44;
    m_DMGPalette[3].blue = 0x03;
    m_DMGPalette[3].alpha = 0xFF;
}

void GearboyCore::InitMemoryRules()
{
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor,
            m_pMemory, m_pVideo, m_pInput, m_pAudio);

    m_pCommonMemoryRule = new CommonMemoryRule(m_pMemory);

    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);

    m_pMBC1MemoryRule = new MBC1MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);

    m_pMultiMBC1MemoryRule = new MultiMBC1MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);

    m_pMBC2MemoryRule = new MBC2MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);

    m_pMBC3MemoryRule = new MBC3MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);

    m_pMBC5MemoryRule = new MBC5MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge, m_pAudio);
}

bool GearboyCore::AddMemoryRules()
{
    m_pMemory->SetIORule(m_pIORegistersMemoryRule);
    m_pMemory->SetCommonRule(m_pCommonMemoryRule);

    Cartridge::CartridgeTypes type = m_pCartridge->GetType();

    bool notSupported = false;

    switch (type)
    {
        case Cartridge::CartridgeNoMBC:
            m_pMemory->SetCurrentRule(m_pRomOnlyMemoryRule);
            break;
        case Cartridge::CartridgeMBC1:
            m_pMemory->SetCurrentRule(m_pMBC1MemoryRule);
            break;
        case Cartridge::CartridgeMBC1Multi:
            m_pMemory->SetCurrentRule(m_pMultiMBC1MemoryRule);
            break;
        case Cartridge::CartridgeMBC2:
            m_pMemory->SetCurrentRule(m_pMBC2MemoryRule);
            break;
        case Cartridge::CartridgeMBC3:
            m_pMemory->SetCurrentRule(m_pMBC3MemoryRule);
            break;
        case Cartridge::CartridgeMBC5:
            m_pMemory->SetCurrentRule(m_pMBC5MemoryRule);
            break;
        case Cartridge::CartridgeNotSupported:
            notSupported = true;
            break;
        default:
            notSupported = true;
    }

    if (!notSupported)
    {
        m_pMemory->GetCurrentRule()->SetRamChangedCallback(m_pRamChangedCallback);
    }

    return !notSupported;
}

void GearboyCore::Reset(bool bCGB)
{
    m_bCGB = bCGB;

    if (m_bCGB)
    {
        Log("Reset: Switching to Game Boy Color");
    }
    else
    {
        Log("Reset: Defaulting to Game Boy DMG");
    }
    
    if (m_bDuringBootROM)
    {
        Log("Boot rom mode");
    }

    m_pMemory->Reset(m_bCGB, m_bDuringBootROM);
    m_pProcessor->Reset(m_bCGB, m_bDuringBootROM);
    m_pVideo->Reset(m_bCGB);
    m_pAudio->Reset(m_bCGB);
    m_pInput->Reset();
    m_pCartridge->UpdateCurrentRTC();
    m_bRTCUpdateCount = 0;

    m_pCommonMemoryRule->Reset(m_bCGB);
    m_pRomOnlyMemoryRule->Reset(m_bCGB);
    m_pMBC1MemoryRule->Reset(m_bCGB);
    m_pMultiMBC1MemoryRule->Reset(m_bCGB);
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
