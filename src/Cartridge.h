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

#include "definitions.h"

class Cartridge
{
public:
    Cartridge();
    ~Cartridge();
    void Init();
    void Reset();
    bool IsValidROM() const;
    bool IsLoadedROM() const;
    int GetType() const;
    int GetRAMSize() const;
    int GetROMSize() const;
    const char* GetName() const;
    int GetTotalSize() const;
    u8* GetTheROM() const;
    bool LoadFromFile(const char* path);
    bool LoadFromBuffer(const u8* buffer, int size);
    int GetVersion() const;
    bool IsSGB() const;
    bool IsCGB() const;
    void UpdateCurrentRTC();
    size_t GetCurrentRTC();

private:
    void GatherMetadata();
    bool LoadFromZipFile(const u8* buffer, int size);

private:
    u8* m_pTheROM;
    int m_iTotalSize;
    char m_szName[16];
    int m_iROMSize;
    int m_iRAMSize;
    int m_iType;
    bool m_bValidROM;
    bool m_bCGB;
    bool m_bSGB;
    int m_iVersion;
    bool m_bLoaded;
    size_t m_RTCCurrentTime;
};

#endif	/* CARTRIDGE_H */
