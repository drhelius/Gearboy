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

#ifndef CORE_H
#define	CORE_H

#include "definitions.h"
#include "Cartridge.h"

class Memory;
class Processor;
class Video;
class Audio;
class Input;
class CommonMemoryRule;
class IORegistersMemoryRule;
class RomOnlyMemoryRule;
class MBC1MemoryRule;
class MBC2MemoryRule;
class MBC3MemoryRule;
class MBC5MemoryRule;
class MultiMBC1MemoryRule;
class MemoryRule;

class GearboyCore
{
public:
    GearboyCore();
    ~GearboyCore();
    void Init(GB_Color_Format pixelFormat = GB_PIXEL_RGB565);
    bool RunToVBlank(u16* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, bool bDMGbuffer = false, bool step = false, bool stopOnBreakpoints = false);
    bool LoadROM(const char* szFilePath, bool forceDMG, Cartridge::CartridgeTypes forceType = Cartridge::CartridgeNotSupported);
    bool LoadROMFromBuffer(const u8* buffer, int size, bool forceDMG, Cartridge::CartridgeTypes forceType = Cartridge::CartridgeNotSupported);
    void SaveMemoryDump();
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
    void Pause(bool paused);
    bool IsPaused();
    void ResetROM(bool forceDMG, Cartridge::CartridgeTypes forceType = Cartridge::CartridgeNotSupported);
    void ResetROMPreservingRAM(bool forceDMG, Cartridge::CartridgeTypes forceType = Cartridge::CartridgeNotSupported);
    void ResetSound();
    void SetSoundSampleRate(int rate);
    void SetSoundVolume(float volume);
    void SetDMGPalette(GB_Color& color1, GB_Color& color2, GB_Color& color3, GB_Color& color4);
    u16* GetDMGInternalPalette();
    void SaveRam();
    void SaveRam(const char* szPath, bool fullPath = false);
    void LoadRam();
    void LoadRam(const char* szPath, bool fullPath = false);
    void SaveState(int index);
    void SaveState(const char* szPath, int index);
    bool SaveState(u8* buffer, size_t& size);
    bool SaveState(std::ostream& stream, size_t& size);
    void LoadState(int index);
    void LoadState(const char* szPath, int index);
    bool LoadState(const u8* buffer, size_t size);
    bool LoadState(std::istream& stream);
    void SetCheat(const char* szCheat);
    void ClearCheats();
    void SetRamModificationCallback(RamChangedCallback callback);
    bool IsCGB();
    Memory* GetMemory();
    Cartridge* GetCartridge();
    Processor* GetProcessor();
    Audio* GetAudio();
    Video* GetVideo();

private:
    void RenderDMGFrame(u16* pFrameBuffer) const;
    void InitDMGPalette();
    void InitMemoryRules();
    bool AddMemoryRules(Cartridge::CartridgeTypes forceType = Cartridge::CartridgeNotSupported);
    void Reset(bool bCGB);

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Video* m_pVideo;
    Audio* m_pAudio;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    CommonMemoryRule* m_pCommonMemoryRule;
    IORegistersMemoryRule* m_pIORegistersMemoryRule;
    RomOnlyMemoryRule* m_pRomOnlyMemoryRule;
    MBC1MemoryRule* m_pMBC1MemoryRule;
    MBC2MemoryRule* m_pMBC2MemoryRule;
    MBC3MemoryRule* m_pMBC3MemoryRule;
    MBC5MemoryRule* m_pMBC5MemoryRule;
    MultiMBC1MemoryRule* m_pMultiMBC1MemoryRule;
    bool m_bCGB;
    bool m_bPaused;
    u16 m_DMGPalette[4];
    bool m_bForceDMG;
    int m_iRTCUpdateCount;
    RamChangedCallback m_pRamChangedCallback;
    GB_Color_Format m_pixelFormat;
};

#endif	/* CORE_H */
