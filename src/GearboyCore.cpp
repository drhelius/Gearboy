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
    InitPointer(m_pRamChangedCallback);
    m_bCGB = false;
    m_bPaused = true;
    m_bForceDMG = false;
    m_iRTCUpdateCount = 0;
}

GearboyCore::~GearboyCore()
{
#ifdef DEBUG_GEARBOY
    if (m_pCartridge->IsLoadedROM() && (strlen(m_pCartridge->GetFilePath()) > 0))
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

void GearboyCore::RunToVBlank(GB_Color* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount)
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
        }

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);

        m_iRTCUpdateCount++;
        if (m_iRTCUpdateCount == 20)
        {
            m_iRTCUpdateCount = 0;
            m_pCartridge->UpdateCurrentRTC();
        }

        if (!m_bCGB)
        {
            RenderDMGFrame(pFrameBuffer);
        }
    }
}

bool GearboyCore::LoadROM(const char* szFilePath, bool forceDMG)
{
#ifdef DEBUG_GEARBOY
    if (m_pCartridge->IsLoadedROM() && (strlen(m_pCartridge->GetFilePath()) > 0))
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

    if (m_pCartridge->LoadFromFile(szFilePath))
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

bool GearboyCore::LoadROMFromBuffer(const u8* buffer, int size, bool forceDMG)
{
    if (m_pCartridge->LoadFromBuffer(buffer, size))
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        bool romTypeOK = AddMemoryRules();

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header.");
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
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB());
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        AddMemoryRules();
    }
}

void GearboyCore::ResetROMPreservingRAM(bool forceDMG)
{
    if (m_pCartridge->IsLoadedROM())
    {
        Log("Resetting preserving RAM...");

        using namespace std;
        stringstream stream;

        m_pMemory->GetCurrentRule()->SaveRam(stream);

        ResetROM(forceDMG);

        stream.seekg(0, stream.end);
        s32 size = (s32)stream.tellg();
        stream.seekg(0, stream.beg);

        m_pMemory->GetCurrentRule()->LoadRam(stream, size);
    }
}

void GearboyCore::ResetSound()
{
    m_pAudio->Reset(m_bCGB);
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

        string path = "";

        if (IsValidPointer(szPath))
        {
            path += szPath;
            path += "/";
            path += m_pCartridge->GetFileName();
        }
        else
        {
            path = m_pCartridge->GetFilePath();
        }

        string::size_type i = path.rfind('.', path.length());

        if (i != string::npos) {
            path.replace(i + 1, 3, "sav");
        }

        Log("Save file: %s", path.c_str());

        ofstream file(path.c_str(), ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Log("RAM saved");
    }
}

void GearboyCore::LoadRam()
{
    LoadRam(NULL);
}

void GearboyCore::LoadRam(const char* szPath)
{
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Loading RAM...");

        using namespace std;

        string sav_path = "";

        if (IsValidPointer(szPath))
        {
            sav_path += szPath;
            sav_path += "/";
            sav_path += m_pCartridge->GetFileName();
        }
        else
        {
            sav_path = m_pCartridge->GetFilePath();
        }

        string rom_path = sav_path;

        string::size_type i = sav_path.rfind('.', sav_path.length());

        if (i != string::npos) {
            sav_path.replace(i + 1, 3, "sav");
        }

        Log("Opening save file: %s", sav_path.c_str());

        ifstream file;

        file.open(sav_path.c_str(), ios::in | ios::binary);

        // check for old .gearboy saves
        if (file.fail())
        {
            Log("Save file doesn't exist");
            string old_sav_file = rom_path + ".gearboy";

            Log("Opening old save file: %s", old_sav_file.c_str());
            file.open(old_sav_file.c_str(), ios::in | ios::binary);
        }

        if (!file.fail())
        {
            file.seekg(0, file.end);
            s32 fileSize = (s32)file.tellg();
            file.seekg(0, file.beg);

            if (m_pMemory->GetCurrentRule()->LoadRam(file, fileSize))
            {
                Log("RAM loaded");
            }
            else
            {
                Log("Save file size incorrect: %d", fileSize);
            }
        }
        else
        {
            Log("Save file doesn't exist");
        }
    }
}

void GearboyCore::SaveState(int index)
{
    SaveState(NULL, index);
}

void GearboyCore::SaveState(const char* szPath, int index)
{
    Log("Creating save state...");

    using namespace std;

    size_t size;
    SaveState(NULL, size);

    u8* buffer = new u8[size];
    string path = "";

    if (IsValidPointer(szPath))
    {
        path += szPath;
        path += "/";
        path += m_pCartridge->GetFileName();
    }
    else
    {
        path = m_pCartridge->GetFilePath();
    }

    string::size_type i = path.rfind('.', path.length());

    if (i != string::npos) {
        path.replace(i + 1, 3, "state");
    }

    std::stringstream sstm;
    sstm << path << index;

    Log("Save state file: %s", sstm.str().c_str());

    ofstream file(sstm.str().c_str(), ios::out | ios::binary);

    SaveState(file, size);

    Log("Save state file created");

    SafeDeleteArray(buffer);
}

bool GearboyCore::SaveState(u8* buffer, size_t& size)
{
    bool ret = false;

    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        stringstream stream;

        if (SaveState(stream, size))
            ret = true;

        if (IsValidPointer(buffer))
        {
            Log("Saving state...");
            memcpy(buffer, stream.str().c_str(), size);
            ret = true;
        }
    }

    return ret;
}

bool GearboyCore::SaveState(std::ostream& stream, size_t& size)
{
    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Gathering save state data...");

        using namespace std;

        m_pMemory->SaveState(stream);
        m_pProcessor->SaveState(stream);
        m_pVideo->SaveState(stream);
        m_pInput->SaveState(stream);
        m_pAudio->SaveState(stream);
        m_pMemory->GetCurrentRule()->SaveState(stream);

        size = static_cast<size_t>(stream.tellp());
        size += (sizeof(u32) * 2);

        u32 header_magic = SAVESTATE_MAGIC;
        size_t header_size = size;

        stream.write(reinterpret_cast<const char*> (&header_magic), sizeof(header_magic));
        stream.write(reinterpret_cast<const char*> (&header_size), sizeof(header_size));

        return true;
    }

    return false;
}

void GearboyCore::LoadState(int index)
{
    LoadState(NULL, index);
}

void GearboyCore::LoadState(const char* szPath, int index)
{
    Log("Loading save state...");

    using namespace std;

    string sav_path = "";

    if (IsValidPointer(szPath))
    {
        sav_path += szPath;
        sav_path += "/";
        sav_path += m_pCartridge->GetFileName();
    }
    else
    {
        sav_path = m_pCartridge->GetFilePath();
    }

    string rom_path = sav_path;

    string::size_type i = sav_path.rfind('.', sav_path.length());

    if (i != string::npos) {
        sav_path.replace(i + 1, 3, "state");
    }

    std::stringstream sstm;
    sstm << sav_path << index;

    Log("Opening save file: %s", sstm.str().c_str());

    ifstream file;

    file.open(sstm.str().c_str(), ios::in | ios::binary);

    if (!file.fail())
    {
        if (LoadState(file))
        {
            Log("Save state loaded");
        }
    }
    else
    {
        Log("Save state file doesn't exist");
    }
}

bool GearboyCore::LoadState(const u8* buffer, size_t size)
{
    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()) && (size > 0) && IsValidPointer(buffer))
    {
        Log("Gathering load state data...");

        using namespace std;

        stringstream stream;

        stream.write(reinterpret_cast<const char*> (buffer), size);

        return LoadState(stream);
    }

    return false;
}

bool GearboyCore::LoadState(std::istream& stream)
{
    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        u32 header_magic = 0;
        u32 header_size = 0;

        stream.seekg(0, ios::end);
        size_t size = static_cast<size_t>(stream.tellg());
        stream.seekg(0, ios::beg);

        stream.seekg(-2 * (sizeof(u32)), ios::end);
        stream.read(reinterpret_cast<char*> (&header_magic), sizeof(header_magic));
        stream.read(reinterpret_cast<char*> (&header_size), sizeof(header_size));
        stream.seekg(0, ios::beg);

        if ((header_magic == SAVESTATE_MAGIC) && (header_size == size))
        {
            Log("Loading state...");

            m_pMemory->LoadState(stream);
            m_pProcessor->LoadState(stream);
            m_pVideo->LoadState(stream);
            m_pInput->LoadState(stream);
            m_pAudio->LoadState(stream);
            m_pMemory->GetCurrentRule()->LoadState(stream);

            return true;
        }
        else
        {
            Log("Invalid save state size or header");
        }
    }

    return false;
}

void GearboyCore::SetRamModificationCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

bool GearboyCore::IsCGB()
{
    return m_bCGB;
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
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor, m_pMemory, m_pVideo, m_pInput, m_pAudio);

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

    m_pMemory->Reset(m_bCGB);
    m_pProcessor->Reset(m_bCGB);
    m_pVideo->Reset(m_bCGB);
    m_pAudio->Reset(m_bCGB);
    m_pInput->Reset();
    m_pCartridge->UpdateCurrentRTC();
    m_iRTCUpdateCount = 0;

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
    if (IsValidPointer(pFrameBuffer))
    {
        int pixels = GAMEBOY_WIDTH * GAMEBOY_HEIGHT;
        const u8* pGameboyFrameBuffer = m_pVideo->GetFrameBuffer();

        for (int i = 0; i < pixels; i++)
        {
            pFrameBuffer[i] = m_DMGPalette[pGameboyFrameBuffer[i]];
        }
    }
}
