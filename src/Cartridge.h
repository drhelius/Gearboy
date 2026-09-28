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

#ifndef CARTRIDGE_H
#define	CARTRIDGE_H

#include <list>
#include <ctime>
#include "definitions.h"
#include "log.h"

class Cartridge
{
public:
    enum CartridgeTypes
    {
        CartridgeNoMBC,
        CartridgeMBC1,
        CartridgeMBC2,
        CartridgeMBC3,
        CartridgeMBC5,
        CartridgeMBC1Multi,
        CartridgeHuC1,
        CartridgeHuC3,
        CartridgeMMM01,
        CartridgeCamera,
        CartridgeMBC7,
        CartridgeTAMA5,
        CartridgeWisdomTree,
        CartridgeM161,
        CartridgeSachenMMC1,
        CartridgeSachenMMC2,
        CartridgePKJD,
        CartridgeBungEMS,
        CartridgePoke2in1,
        CartridgeMBC6,
        CartridgeRocket,
        CartridgeBHGOS,
        CartridgeLiCheng,
        CartridgeNTNew,
        CartridgeGGB81,
        CartridgeHitek,
        CartridgeVF001,
        CartridgeVF001A,
        CartridgeSintax,
        CartridgeNTOld1,
        CartridgeNTOld2,
        CartridgeNotSupported
    };

    struct GameGenieCode
    {
        int address;
        u8 old_value;
    };

public:
    Cartridge();
    ~Cartridge();
    void Init();
    void Reset();
    bool IsValidROM() const;
    bool IsLoadedROM() const;
    INLINE CartridgeTypes GetType() const;
    INLINE int GetRAMSize() const;
    int GetROMSize() const;
    INLINE int GetROMBankCount() const;
    INLINE int GetRAMBankCount() const;
    const char* GetName() const;
    const char* GetFilePath() const;
    const char* GetFileName() const;
    const char* GetFileDirectory() const;
    int GetTotalSize() const;
    u32 GetCRC() const;
    bool IsBarcodeBoySupported() const;
    bool IsBootLogoSwapDisabled() const;
    bool HasRam() const;
    bool HasBattery() const;
    INLINE u8* GetTheROM() const;
    bool LoadFromFile(const char* path, bool softpatching = false);
    bool LoadFromBuffer(const u8* buffer, int size);
    bool IsSoftpatchApplied() const;
    const char* GetSoftpatchPath() const;
    int GetVersion() const;
    bool IsSGB() const;
    bool IsCGB() const;
    void UpdateCurrentRTC();
    time_t GetCurrentRTC();
    INLINE bool IsRTCPresent() const;
    bool IsRumblePresent() const;
    bool IsMBC30() const;
    void SetGameGenieCheat(const char* szCheat);
    void ClearGameGenieCheats();

private:
    bool GatherMetadata(u32 crc);
    void GetInfoFromDB(u32 crc);
    bool LoadFromZipFile(const u8* buffer, int size, bool softpatching);
    bool LoadFromBufferWithSoftpatch(const u8* buffer, int size, bool softpatching);
    void CheckCartridgeType(int type);
    bool IsBungEMSCartridge() const;
    bool IsSachenMMC1Cartridge() const;
    bool IsSachenMMC2Cartridge() const;
    bool IsWisdomTreeCartridge(int type) const;
    bool IsLiChengCartridge() const;

private:
    u8* m_pTheROM;
    int m_iTotalSize;
    char m_szName[16];
    int m_iROMSize;
    int m_iRAMSize;
    CartridgeTypes m_Type;
    bool m_bValidROM;
    bool m_bCGB;
    bool m_bSGB;
    int m_iVersion;
    bool m_bLoaded;
    time_t m_RTCCurrentTime;
    bool m_bBattery;
    char m_szFilePath[512];
    char m_szFileName[512];
    char m_szFileDirectory[512];
    bool m_bRTCPresent;
    bool m_bRumblePresent;
    bool m_bMBC30;
    int m_iRAMBankCount;
    int m_iROMBankCount;
    bool m_softpatch_applied;
    char m_softpatch_path[4096];
    std::list<GameGenieCode> m_GameGenieList;
    u32 m_iCRC;
    int m_iFeatures;
};

INLINE Cartridge::CartridgeTypes Cartridge::GetType() const
{
    return m_Type;
}

INLINE int Cartridge::GetRAMSize() const
{
    return m_iRAMSize;
}

INLINE int Cartridge::GetROMBankCount() const
{
    return m_iROMBankCount;
}

INLINE int Cartridge::GetRAMBankCount() const
{
    return m_iRAMBankCount;
}

INLINE u8* Cartridge::GetTheROM() const
{
    return m_pTheROM;
}

INLINE bool Cartridge::IsRTCPresent() const
{
    return m_bRTCPresent;
}

#endif	/* CARTRIDGE_H */
