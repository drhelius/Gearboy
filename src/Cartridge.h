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
private:
    void GatherMetadata();
private:
    u8* m_pTheROM;
    int m_iTotalSize;
    char m_szName[MAX_STRING_SIZE];
    int m_iROMSize;
    int m_iRAMSize;
    int m_iType;
    bool m_bValidROM;
    bool m_bCGB;
    bool m_bSGB;
    int m_iVersion;
    bool m_bLoaded;
};

#endif	/* CARTRIDGE_H */

