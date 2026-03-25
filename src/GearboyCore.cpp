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

#include <math.h>
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
#include "TraceLogger.h"
#include "common.h"

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
    InitPointer(m_trace_logger);
    m_bCGB = false;
    m_bGBA = false;
    m_bPaused = false;
    m_bForceDMG = false;
    m_iRTCUpdateCount = 0;
    m_pixelFormat = GB_PIXEL_RGB565;
    m_bColorCorrectionEnabled = false;
    m_pSaveStateFrameBuffer = NULL;
    m_master_clock_cycles = 0;
}

GearboyCore::~GearboyCore()
{
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
    SafeDelete(m_trace_logger);
}

void GearboyCore::Init(GB_Color_Format pixelFormat)
{
    Log("Loading %s core %s by Ignacio Sanchez", GEARBOY_TITLE, GEARBOY_VERSION);

    m_pixelFormat = pixelFormat;

    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pAudio = new Audio();
    m_pInput = new Input(m_pMemory, m_pProcessor);
    m_pCartridge = new Cartridge();
    m_trace_logger = new TraceLogger();

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pVideo->Init();
    m_pAudio->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    m_pProcessor->SetTraceLogger(m_trace_logger);
    m_pVideo->SetTraceLogger(m_trace_logger);

    InitMemoryRules();
    InitDMGPalette();
}

bool GearboyCore::RunToVBlank(u16* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, bool bDMGbuffer, GB_Debug_Run* debug)
{
    bool breakpoint_result = false;

    if (!m_bPaused && m_pCartridge->IsLoadedROM())
    {
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
        bool debug_enable = false;

        if (IsValidPointer(debug))
        {
            debug_enable = true;
            m_pProcessor->EnableBreakpoints(debug->stop_on_breakpoint, debug->stop_on_irq);
        }

        bool vblank = false;
        int totalClocks = 0;

        do
        {
            unsigned int clockCycles = m_pProcessor->RunFor(1);

            m_pProcessor->UpdateTimers(clockCycles);
            m_pProcessor->UpdateSerial(clockCycles);

            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer, m_pixelFormat);
            m_pAudio->Tick(clockCycles);
            m_pInput->Tick(clockCycles);
            totalClocks += clockCycles;
            m_master_clock_cycles += clockCycles;

            if (debug_enable)
            {
                if (debug->step_debugger && !m_pProcessor->DuringOpCode() && !m_pProcessor->Halted())
                    vblank = true;

                if (m_pProcessor->MemoryBreakpointHit())
                    vblank = true;

                if (m_pProcessor->BreakpointHit())
                    vblank = true;

                if (debug->stop_on_run_to_breakpoint && m_pProcessor->RunToBreakpointHit())
                    vblank = true;
            }

            if (totalClocks > GAMEBOY_CLOCKS_SAFE_LIMIT)
                vblank = true;
        }
        while (!vblank);

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);

        m_iRTCUpdateCount++;
        if (m_iRTCUpdateCount == 20)
        {
            m_iRTCUpdateCount = 0;
            m_pCartridge->UpdateCurrentRTC();
        }

        if (!m_bCGB && !bDMGbuffer)
        {
            RenderDMGFrame(pFrameBuffer);
        }
        else if (m_bCGB && m_bColorCorrectionEnabled)
        {
            ApplyColorCorrection(pFrameBuffer, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
        }

        breakpoint_result = m_pProcessor->BreakpointHit() || m_pProcessor->RunToBreakpointHit();
#else
        UNUSED(debug);
        bool vblank = false;
        int totalClocks = 0;

        do
        {
            #ifdef PERFORMANCE
                unsigned int clockCycles = m_pProcessor->RunFor(75);
            #else
                unsigned int clockCycles = m_pProcessor->RunFor(1);
            #endif

            m_pProcessor->UpdateTimers(clockCycles);
            m_pProcessor->UpdateSerial(clockCycles);

            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer, m_pixelFormat);
            m_pAudio->Tick(clockCycles);
            m_pInput->Tick(clockCycles);
            totalClocks += clockCycles;
            m_master_clock_cycles += clockCycles;

            if (totalClocks > GAMEBOY_CLOCKS_SAFE_LIMIT)
                vblank = true;
        }
        while (!vblank);

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);

        m_iRTCUpdateCount++;
        if (m_iRTCUpdateCount == 20)
        {
            m_iRTCUpdateCount = 0;
            m_pCartridge->UpdateCurrentRTC();
        }

        if (!m_bCGB && !bDMGbuffer)
        {
            RenderDMGFrame(pFrameBuffer);
        }
        else if (m_bCGB && m_bColorCorrectionEnabled)
        {
            ApplyColorCorrection(pFrameBuffer, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
        }
#endif
    }

    return breakpoint_result;
}

bool GearboyCore::LoadROM(const char* szFilePath, bool forceDMG, Cartridge::CartridgeTypes forceType, bool forceGBA)
{
    if (m_pCartridge->LoadFromFile(szFilePath))
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB(), forceGBA);
        m_pMemory->ResetDisassemblerRecords();
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        bool romTypeOK = AddMemoryRules(forceType);
#ifndef GEARBOY_DISABLE_DISASSEMBLER
        m_pProcessor->DisassembleNextOPCode();
#endif

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);
        }

        return romTypeOK;
    }
    else
        return false;
}

bool GearboyCore::LoadROMFromBuffer(const u8* buffer, int size, bool forceDMG, Cartridge::CartridgeTypes forceType, bool forceGBA)
{
    if (m_pCartridge->LoadFromBuffer(buffer, size))
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB(), forceGBA);
        m_pMemory->ResetDisassemblerRecords();
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        bool romTypeOK = AddMemoryRules(forceType);

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

Processor* GearboyCore::GetProcessor()
{
    return m_pProcessor;
}

Audio* GearboyCore::GetAudio()
{
    return m_pAudio;
}

Video* GearboyCore::GetVideo()
{
    return m_pVideo;
}

TraceLogger* GearboyCore::GetTraceLogger()
{
    return m_trace_logger;
}

u64 GearboyCore::GetMasterClockCycles()
{
    return m_master_clock_cycles;
}

bool GearboyCore::GetRuntimeInfo(GB_RuntimeInfo& runtime_info)
{
    runtime_info.screen_width = GAMEBOY_WIDTH;
    runtime_info.screen_height = GAMEBOY_HEIGHT;
    return m_pCartridge->IsLoadedROM();
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

void GearboyCore::ResetROM(bool forceDMG, Cartridge::CartridgeTypes forceType, bool forceGBA)
{
    if (m_pCartridge->IsLoadedROM())
    {
        m_bForceDMG = forceDMG;
        Reset(m_bForceDMG ? false : m_pCartridge->IsCGB(), forceGBA);
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
        AddMemoryRules(forceType);
#ifndef GEARBOY_DISABLE_DISASSEMBLER
        m_pProcessor->DisassembleNextOPCode();
#endif
    }
}

void GearboyCore::ResetROMPreservingRAM(bool forceDMG, Cartridge::CartridgeTypes forceType, bool forceGBA)
{
    if (m_pCartridge->IsLoadedROM())
    {
        Debug("Resetting preserving RAM...");

        using namespace std;
        stringstream stream;

        m_pMemory->GetCurrentRule()->SaveRam(stream);

        ResetROM(forceDMG, forceType, forceGBA);

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

void GearboyCore::SetSoundVolume(float volume)
{
    m_pAudio->SetVolume(volume);
}

u16* GearboyCore::GetDMGInternalPalette()
{
    return m_DMGPalette;
}

void GearboyCore::SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3,
        GB_Color& color4)
{
    bool format_565 = (m_pixelFormat == GB_PIXEL_RGB565) || (m_pixelFormat == GB_PIXEL_BGR565);
    bool order_RGB = (m_pixelFormat == GB_PIXEL_RGB565) || (m_pixelFormat == GB_PIXEL_RGB555);

    int multiplier = format_565 ? 63 : 31;
    int shift = format_565 ? 11 : 10;

    if (order_RGB)
    {
        m_DMGPalette[0] = (((color1.red * 31) / 255) << shift ) | (((color1.green * multiplier) / 255) << 5 ) | ((color1.blue * 31) / 255);
        m_DMGPalette[1] = (((color2.red * 31) / 255) << shift ) | (((color2.green * multiplier) / 255) << 5 ) | ((color2.blue * 31) / 255);
        m_DMGPalette[2] = (((color3.red * 31) / 255) << shift ) | (((color3.green * multiplier) / 255) << 5 ) | ((color3.blue * 31) / 255);
        m_DMGPalette[3] = (((color4.red * 31) / 255) << shift ) | (((color4.green * multiplier) / 255) << 5 ) | ((color4.blue * 31) / 255);        
    }
    else
    {
        m_DMGPalette[0] = (((color1.blue * 31) / 255) << shift ) | (((color1.green * multiplier) / 255) << 5 ) | ((color1.red * 31) / 255);
        m_DMGPalette[1] = (((color2.blue * 31) / 255) << shift ) | (((color2.green * multiplier) / 255) << 5 ) | ((color2.red * 31) / 255);
        m_DMGPalette[2] = (((color3.blue * 31) / 255) << shift ) | (((color3.green * multiplier) / 255) << 5 ) | ((color3.red * 31) / 255);
        m_DMGPalette[3] = (((color4.blue * 31) / 255) << shift ) | (((color4.green * multiplier) / 255) << 5 ) | ((color4.red * 31) / 255);
    }   

    if (!format_565)
    {
        m_DMGPalette[0] |= 0x8000;
        m_DMGPalette[1] |= 0x8000;
        m_DMGPalette[2] |= 0x8000;
        m_DMGPalette[3] |= 0x8000;
    }
}

void GearboyCore::SaveRam()
{
    SaveRam(NULL);
}

void GearboyCore::SaveRam(const char* szPath, bool fullPath)
{
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Debug("Saving RAM...");

        using namespace std;

        string path = "";

        if (IsValidPointer(szPath))
        {
            path += szPath;

            if (!fullPath)
            {
                path += "/";
                path += m_pCartridge->GetFileName();
            }
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

        ofstream file;
        open_ofstream_utf8(file, path.c_str(), ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Debug("RAM saved");
    }
}

void GearboyCore::LoadRam()
{
    LoadRam(NULL);
}

void GearboyCore::LoadRam(const char* szPath, bool fullPath)
{
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Debug("Loading RAM...");

        using namespace std;

        string sav_path = "";

        if (IsValidPointer(szPath))
        {
            sav_path += szPath;

            if (!fullPath)
            {
                sav_path += "/";
                sav_path += m_pCartridge->GetFileName();
            }
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

        open_ifstream_utf8(file, sav_path.c_str(), ios::in | ios::binary);

        // check for old .gearboy saves
        if (file.fail())
        {
            Log("Save file doesn't exist");
            string old_sav_file = rom_path + ".gearboy";

            Log("Opening old save file: %s", old_sav_file.c_str());
            open_ifstream_utf8(file, old_sav_file.c_str(), ios::in | ios::binary);
        }

        if (!file.fail())
        {
            file.seekg(0, file.end);
            s32 fileSize = (s32)file.tellg();
            file.seekg(0, file.beg);

            if (m_pMemory->GetCurrentRule()->LoadRam(file, fileSize))
            {
                Debug("RAM loaded");
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
    SaveState(NULL, index, false);
}

void GearboyCore::SaveState(const char* szPath, int index)
{
    SaveState(szPath, index, false);
}

void GearboyCore::SetFrameBuffer(u8* frame_buffer)
{
    m_pSaveStateFrameBuffer = frame_buffer;
}

std::string GearboyCore::GetSaveStatePath(const char* path, int index)
{
    if (index < 0)
        return path;

    using namespace std;
    string full_path;

    if (IsValidPointer(path))
    {
        full_path = path;
        full_path += "/";
        full_path += m_pCartridge->GetFileName();
    }
    else
        full_path = m_pCartridge->GetFilePath();

    string::size_type dot_index = full_path.rfind('.');

    if (dot_index != string::npos)
        full_path.replace(dot_index + 1, full_path.length() - dot_index - 1, "state");

    stringstream ss;
    ss << index;
    full_path += ss.str();

    return full_path;
}

bool GearboyCore::SaveState(const char* path, int index, bool screenshot)
{
    using namespace std;

    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    string full_path = GetSaveStatePath(path, index);
    Debug("Saving state to %s...", full_path.c_str());

    ofstream stream;
    open_ofstream_utf8(stream, full_path.c_str(), ios::out | ios::binary);

    size_t size;
    bool ret = SaveState(stream, size, screenshot);
    if (ret)
        Log("Saved state to %s", full_path.c_str());
    else
        Log("Failed to save state to %s", full_path.c_str());
    return ret;
}

bool GearboyCore::SaveState(u8* buffer, size_t& size, bool screenshot)
{
    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    bool ret = false;

    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        stringstream stream;

        if (SaveState(stream, size, screenshot))
            ret = true;

        if (IsValidPointer(buffer))
        {
            Log("Saving state to buffer [%d bytes]...", size);
            memcpy(buffer, stream.str().c_str(), size);
            ret = true;
        }
    }
    else
    {
        Log("Invalid rom or memory rule.");
    }

    return ret;
}

bool GearboyCore::SaveState(std::ostream& stream, size_t& size, bool screenshot)
{
    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Debug("Serializing save state...");

        using namespace std;

        m_pMemory->SaveState(stream);
        m_pProcessor->SaveState(stream);
        m_pVideo->SaveState(stream);
        m_pInput->SaveState(stream);
        m_pAudio->SaveState(stream);
        m_pMemory->GetCurrentRule()->SaveState(stream);

#if defined(__LIBRETRO__)
        GB_SaveState_Header_Libretro header;
        header.magic = GB_SAVESTATE_MAGIC;
        header.version = GB_SAVESTATE_VERSION;
        Debug("Save state header magic: 0x%08x", header.magic);
        Debug("Save state header version: %d", header.version);
#else
        GB_SaveState_Header header;
        memset(&header, 0, sizeof(header));
        header.magic = GB_SAVESTATE_MAGIC;
        header.version = GB_SAVESTATE_VERSION;

        header.timestamp = (s64)time(NULL);
        strncpy(header.rom_name, m_pCartridge->GetFileName(), sizeof(header.rom_name) - 1);
        header.rom_name[sizeof(header.rom_name) - 1] = 0;
        header.rom_crc = 0;
        strncpy(header.emu_build, GEARBOY_VERSION, sizeof(header.emu_build) - 1);
        header.emu_build[sizeof(header.emu_build) - 1] = 0;

        Debug("Save state header magic: 0x%08x", header.magic);
        Debug("Save state header version: %d", header.version);
        Debug("Save state header rom name: %s", header.rom_name);
        Debug("Save state header emu build: %s", header.emu_build);

        if (screenshot && IsValidPointer(m_pSaveStateFrameBuffer))
        {
            header.screenshot_width = GAMEBOY_WIDTH;
            header.screenshot_height = GAMEBOY_HEIGHT;
            header.screenshot_size = GAMEBOY_WIDTH * GAMEBOY_HEIGHT * 3;
            stream.write(reinterpret_cast<const char*>(m_pSaveStateFrameBuffer), header.screenshot_size);
        }
        else
        {
            header.screenshot_size = 0;
            header.screenshot_width = 0;
            header.screenshot_height = 0;
        }

        Debug("Save state header screenshot size: %d", header.screenshot_size);
#endif

        size = static_cast<size_t>(stream.tellp());
        size += sizeof(header);

#if !defined(__LIBRETRO__)
        header.size = static_cast<u32>(size);
        Debug("Save state header size: %d", header.size);
#endif

        stream.write(reinterpret_cast<const char*>(&header), sizeof(header));

        Log("Save state size: %d", static_cast<int>(size));
        return true;
    }

    Log("Invalid rom or memory rule.");
    return false;
}

void GearboyCore::LoadState(int index)
{
    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return;
    }

    Log("Loading save state %d...", index);
    LoadState(NULL, index, false);
    Log("State %d file loaded", index);
}

void GearboyCore::LoadState(const char* szPath, int index)
{
    LoadState(szPath, index, false);
}

bool GearboyCore::LoadState(const char* path, int index, bool)
{
    using namespace std;

    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    bool ret = false;
    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (!stream.fail())
    {
        ret = LoadState(stream);
        if (ret)
            Log("Loaded state from %s", full_path.c_str());
        else
            Log("Failed to load state from %s", full_path.c_str());
    }
    else
    {
        Log("Save state file doesn't exist: %s", full_path.c_str());
    }

    stream.close();
    return ret;
}

bool GearboyCore::LoadState(const u8* buffer, size_t size)
{
    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    if (m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule()) && (size > 0) && IsValidPointer(buffer))
    {
        Debug("Loading state from buffer [%d bytes]...", size);

        using namespace std;

        stringstream stream;
        stream.write(reinterpret_cast<const char*>(buffer), size);

        return LoadState(stream);
    }

    Log("Invalid rom or memory rule.");
    return false;
}

bool GearboyCore::LoadState(std::istream& stream)
{
    if (m_pMemory->IsBootromRegistryEnabled())
    {
        Debug("Save states disabled when running bootrom");
        return false;
    }

    if (!(m_pCartridge->IsLoadedROM() && IsValidPointer(m_pMemory->GetCurrentRule())))
    {
        Log("Invalid rom or memory rule");
        return false;
    }

    using namespace std;

    stream.seekg(0, ios::end);
    size_t size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, ios::beg);

#if defined(__LIBRETRO__)
    GB_SaveState_Header_Libretro header;
#else
    GB_SaveState_Header header;
#endif

    if (size < sizeof(header))
    {
        Log("Save state too small for current header (%d bytes), trying legacy format...", static_cast<int>(size));
        return LoadStateLegacy(stream, size);
    }

    stream.seekg(size - sizeof(header), ios::beg);
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    stream.seekg(0, ios::beg);

    Debug("Load state header magic: 0x%08x", header.magic);
    Debug("Load state header version: %d", header.version);

    if (header.magic != GB_SAVESTATE_MAGIC || header.version < GB_SAVESTATE_MIN_VERSION || header.version > GB_SAVESTATE_VERSION)
    {
        Log("Save state header does not match current version, trying legacy format...");
        return LoadStateLegacy(stream, size);
    }

#if !defined(__LIBRETRO__)
    Debug("Load state header size: %d", header.size);
    Debug("Load state header rom name: %s", header.rom_name);
    Debug("Load state header emu build: %s", header.emu_build);
    Debug("Load state header screenshot size: %d", header.screenshot_size);

    if (header.size != size)
    {
        Log("Invalid save state size: %d (expected %d)", header.size, static_cast<int>(size));
        return false;
    }
#endif

    Debug("Loading state...");

    m_pMemory->LoadState(stream);
    m_pProcessor->LoadState(stream);
    m_pVideo->LoadState(stream);
    m_pInput->LoadState(stream);
    m_pAudio->LoadState(stream);
    m_pMemory->GetCurrentRule()->LoadState(stream);

    return true;
}

bool GearboyCore::LoadStateLegacy(std::istream& stream, size_t size)
{
    using namespace std;

    if (size < (2 * sizeof(u32)))
    {
        Log("Save state too small for legacy header (%d bytes)", static_cast<int>(size));
        return false;
    }

    u32 legacy_magic = 0;
    u32 legacy_size = 0;

    stream.seekg(size - (2 * sizeof(u32)), ios::beg);
    stream.read(reinterpret_cast<char*>(&legacy_magic), sizeof(legacy_magic));
    stream.read(reinterpret_cast<char*>(&legacy_size), sizeof(legacy_size));
    stream.seekg(0, ios::beg);

    Debug("Load state legacy magic: 0x%08x", legacy_magic);
    Debug("Load state legacy size: %d", legacy_size);

    if (legacy_magic != SAVESTATE_MAGIC)
    {
        Log("Invalid legacy save state magic: 0x%08x", legacy_magic);
        return false;
    }

    if (legacy_size != size)
    {
        Log("Invalid legacy save state size: %d (expected %d)", legacy_size, static_cast<int>(size));
        return false;
    }

    Log("Loading legacy save state (%d bytes)...", static_cast<int>(size));

    m_pMemory->LoadState(stream);
    m_pProcessor->LoadState(stream);
    m_pVideo->LoadState(stream);
    m_pInput->LoadState(stream);
    m_pAudio->LoadState(stream);
    m_pMemory->GetCurrentRule()->LoadState(stream);

    return true;
}

bool GearboyCore::GetSaveStateHeader(int index, const char* path, GB_SaveState_Header* header)
{
    using namespace std;

    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state header from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (stream.fail())
    {
        Debug("Savestate file doesn't exist %s", full_path.c_str());
        stream.close();
        return false;
    }

    stream.seekg(0, ios::end);
    size_t savestate_size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, ios::beg);

    if (savestate_size < sizeof(GB_SaveState_Header))
    {
        // Try legacy format (8-byte footer: magic + size)
        if (savestate_size < (2 * sizeof(u32)))
        {
            stream.close();
            return false;
        }

        u32 legacy_magic = 0;
        u32 legacy_size = 0;

        stream.seekg(savestate_size - (2 * sizeof(u32)), ios::beg);
        stream.read(reinterpret_cast<char*>(&legacy_magic), sizeof(legacy_magic));
        stream.read(reinterpret_cast<char*>(&legacy_size), sizeof(legacy_size));
        stream.close();

        if (legacy_magic != SAVESTATE_MAGIC || legacy_size != savestate_size)
            return false;

        memset(header, 0, sizeof(GB_SaveState_Header));
        header->magic = legacy_magic;
        header->version = GB_SAVESTATE_LEGACY_VERSION;
        header->size = legacy_size;
        strncpy(header->rom_name, m_pCartridge->GetFileName(), sizeof(header->rom_name) - 1);
        header->rom_name[sizeof(header->rom_name) - 1] = 0;
        return true;
    }

    stream.seekg(savestate_size - sizeof(GB_SaveState_Header), ios::beg);
    stream.read(reinterpret_cast<char*>(header), sizeof(GB_SaveState_Header));
    stream.close();

    if (header->magic != GB_SAVESTATE_MAGIC)
    {
        // Try legacy format
        ifstream stream2;
        open_ifstream_utf8(stream2, full_path.c_str(), ios::in | ios::binary);

        u32 legacy_magic = 0;
        u32 legacy_size = 0;

        stream2.seekg(savestate_size - (2 * sizeof(u32)), ios::beg);
        stream2.read(reinterpret_cast<char*>(&legacy_magic), sizeof(legacy_magic));
        stream2.read(reinterpret_cast<char*>(&legacy_size), sizeof(legacy_size));
        stream2.close();

        if (legacy_magic != SAVESTATE_MAGIC || legacy_size != savestate_size)
            return false;

        memset(header, 0, sizeof(GB_SaveState_Header));
        header->magic = legacy_magic;
        header->version = GB_SAVESTATE_LEGACY_VERSION;
        header->size = legacy_size;
        strncpy(header->rom_name, m_pCartridge->GetFileName(), sizeof(header->rom_name) - 1);
        header->rom_name[sizeof(header->rom_name) - 1] = 0;
        return true;
    }

    return true;
}

bool GearboyCore::GetSaveStateScreenshot(int index, const char* path, GB_SaveState_Screenshot* screenshot)
{
    using namespace std;

    if (!IsValidPointer(screenshot->data) || (screenshot->size == 0))
    {
        Log("Invalid save state screenshot buffer");
        return false;
    }

    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state screenshot from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (stream.fail())
    {
        Log("Savestate file doesn't exist %s", full_path.c_str());
        stream.close();
        return false;
    }

    GB_SaveState_Header header;
    GetSaveStateHeader(index, path, &header);

    if (header.screenshot_size == 0)
    {
        Debug("No screenshot data");
        stream.close();
        return false;
    }

    if (screenshot->size < header.screenshot_size)
    {
        Log("Invalid screenshot buffer size %d < %d", screenshot->size, header.screenshot_size);
        stream.close();
        return false;
    }

    screenshot->size = header.screenshot_size;
    screenshot->width = header.screenshot_width;
    screenshot->height = header.screenshot_height;

    Debug("Screenshot size: %d bytes", screenshot->size);

    if (header.size < sizeof(header) + screenshot->size)
    {
        Log("Invalid screenshot offset");
        stream.close();
        return false;
    }

    stream.seekg(header.size - sizeof(header) - screenshot->size, ios::beg);
    stream.read(reinterpret_cast<char*>(screenshot->data), screenshot->size);
    stream.close();

    return true;
}

void GearboyCore::SetCheat(const char* szCheat)
{
    std::string s = szCheat;
    if ((s.length() == 7) || (s.length() == 11))
    {
        m_pCartridge->SetGameGenieCheat(szCheat);
        if (m_pCartridge->IsLoadedROM())
            m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
    }
    else
    {
        m_pProcessor->SetGameSharkCheat(szCheat);
    }
}

void GearboyCore::ClearCheats()
{
    m_pCartridge->ClearGameGenieCheats();
    m_pProcessor->ClearGameSharkCheats();
    if (m_pCartridge->IsLoadedROM())
        m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
}

void GearboyCore::SetRamModificationCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

bool GearboyCore::IsCGB()
{
    return m_bCGB;
}

bool GearboyCore::IsGBA()
{
    return m_bGBA;
}

void GearboyCore::EnableColorCorrection(bool enabled)
{
    m_bColorCorrectionEnabled = enabled;
}

void GearboyCore::ApplyColorCorrection(u16* pFrameBuffer, int size)
{
    if (!IsValidPointer(pFrameBuffer))
        return;

    const float kGamma = 0.6f;
    const float kR1 = 16.0f;
    const float kR2 = 0.0f;
    const float kR3 = 0.0f;
    const float kG1 = 0.0f;
    const float kG2 = 13.0f;
    const float kG3 = 4.0f;
    const float kB1 = 0.0f;
    const float kB2 = 1.0f;
    const float kB3 = 16.0f;
    const float kLinearGamma = 2.2f;
    const float kOutputGamma = (1.0f / kLinearGamma) * kGamma;
    const float kRScale = 255.0f / 31.0f;
    const float kBScale = 255.0f / 31.0f;

    bool format_565 = (m_pixelFormat == GB_PIXEL_RGB565) || (m_pixelFormat == GB_PIXEL_BGR565);
    bool order_RGB = (m_pixelFormat == GB_PIXEL_RGB565) || (m_pixelFormat == GB_PIXEL_RGB555);

    int r_shift = format_565 ? 11 : 10;
    int g_shift = 5;
    int g_mask = format_565 ? 0x3F : 0x1F;
    int g_max = format_565 ? 63 : 31;
    const float kGScale = 255.0f / (float)g_max;

    for (int i = 0; i < size; i++)
    {
        u16 color = pFrameBuffer[i];
        float r8, g8, b8;

        if (order_RGB)
        {
            r8 = ((color >> r_shift) & 0x1F) * kRScale;
            g8 = ((color >> g_shift) & g_mask) * kGScale;
            b8 = (color & 0x1F) * kBScale;
        }
        else
        {
            b8 = ((color >> r_shift) & 0x1F) * kBScale;
            g8 = ((color >> g_shift) & g_mask) * kGScale;
            r8 = (color & 0x1F) * kRScale;
        }

        float r_lin = to_linear(r8, kLinearGamma);
        float g_lin = to_linear(g8, kLinearGamma);
        float b_lin = to_linear(b8, kLinearGamma);

        float r_out = (r_lin * kR1 + g_lin * kR2 + b_lin * kR3) / 16.0f;
        float g_out = (r_lin * kG1 + g_lin * kG2 + b_lin * kG3) / 16.0f;
        float b_out = (r_lin * kB1 + g_lin * kB2 + b_lin * kB3) / 16.0f;

        r_out = to_gamma(r_out, kOutputGamma);
        g_out = to_gamma(g_out, kOutputGamma);
        b_out = to_gamma(b_out, kOutputGamma);

        u16 r_final = (u16)((CLAMP(r_out, 0.0f, 255.0f) / 255.0f) * 31.0f + 0.5f);
        u16 g_final = (u16)((CLAMP(g_out, 0.0f, 255.0f) / 255.0f) * (float)g_max + 0.5f);
        u16 b_final = (u16)((CLAMP(b_out, 0.0f, 255.0f) / 255.0f) * 31.0f + 0.5f);

        if (order_RGB)
        {
            pFrameBuffer[i] = (r_final << r_shift) | (g_final << g_shift) | b_final;
        }
        else
        {
            pFrameBuffer[i] = (b_final << r_shift) | (g_final << g_shift) | r_final;
        }

        if (!format_565)
        {
            pFrameBuffer[i] |= 0x8000;
        }
    }
}

void GearboyCore::InitDMGPalette()
{
    GB_Color color[4];

    color[0].red = 0x87;
    color[0].green = 0x96;
    color[0].blue = 0x03;

    color[1].red = 0x4d;
    color[1].green = 0x6b;
    color[1].blue = 0x03;

    color[2].red = 0x2b;
    color[2].green = 0x55;
    color[2].blue = 0x03;

    color[3].red = 0x14;
    color[3].green = 0x44;
    color[3].blue = 0x03;

    SetDMGPalette(color[0], color[1], color[2], color[3]);
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

    m_pMemory->SetCurrentRule(m_pRomOnlyMemoryRule);
    m_pMemory->SetIORule(m_pIORegistersMemoryRule);
    m_pMemory->SetCommonRule(m_pCommonMemoryRule);

    m_pIORegistersMemoryRule->SetTraceLogger(m_trace_logger);
    m_pRomOnlyMemoryRule->SetTraceLogger(m_trace_logger);
    m_pMBC1MemoryRule->SetTraceLogger(m_trace_logger);
    m_pMultiMBC1MemoryRule->SetTraceLogger(m_trace_logger);
    m_pMBC2MemoryRule->SetTraceLogger(m_trace_logger);
    m_pMBC3MemoryRule->SetTraceLogger(m_trace_logger);
    m_pMBC5MemoryRule->SetTraceLogger(m_trace_logger);
}

bool GearboyCore::AddMemoryRules(Cartridge::CartridgeTypes forceType)
{
    m_pMemory->SetIORule(m_pIORegistersMemoryRule);
    m_pMemory->SetCommonRule(m_pCommonMemoryRule);

    Cartridge::CartridgeTypes type = m_pCartridge->GetType();

    bool notSupported = false;

    if (forceType != Cartridge::CartridgeNotSupported)
        type = forceType;

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

void GearboyCore::Reset(bool bCGB, bool bGBA)
{
    m_bCGB = bCGB;
    m_bGBA = bGBA;

    if (m_bGBA && m_bCGB)
    {
        Log("Reset: Switching to Game Boy Advance");
    }
    else if (m_bCGB)
    {
        Log("Reset: Switching to Game Boy Color");
    }
    else
    {
        Log("Reset: Defaulting to Game Boy DMG");
    }

    m_pMemory->Reset(m_bCGB);
    m_pProcessor->Reset(m_bCGB, m_bGBA);
    m_pVideo->Reset(m_bCGB);
    m_pAudio->Reset(m_bCGB);
    m_pInput->Reset();
    m_pCartridge->UpdateCurrentRTC();
    m_iRTCUpdateCount = 0;
    m_master_clock_cycles = 0;

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

void GearboyCore::RenderDMGFrame(u16* pFrameBuffer) const
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
