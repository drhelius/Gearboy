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

#include "Cartridge.h"

Cartridge::Cartridge()
{
    InitPointer(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_iType = 0;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
}

Cartridge::~Cartridge()
{
    SafeDeleteArray(m_pTheROM);
}

void Cartridge::Init()
{
    Reset();
}

void Cartridge::Reset()
{
    SafeDeleteArray(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_iType = 0;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
}

bool Cartridge::IsValidROM() const
{
    return m_bValidROM;
}

bool Cartridge::IsLoadedROM() const
{
    return m_bLoaded;
}

int Cartridge::GetType() const
{
    return m_iType;
}

int Cartridge::GetRAMSize() const
{
    return m_iRAMSize;
}

int Cartridge::GetROMSize() const
{
    return m_iROMSize;
}

const char* Cartridge::GetName() const
{
    return m_szName;
}

int Cartridge::GetTotalSize() const
{
    return m_iTotalSize;
}

u8* Cartridge::GetTheROM() const
{
    return m_pTheROM;
}

bool Cartridge::LoadFromFile(const char* path)
{
    Reset();

    using namespace std;

    Log("Loading %s...", path);

    ifstream file(path, ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
        int size = static_cast<int> (file.tellg());
        char* memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        m_bLoaded = LoadFromBuffer(reinterpret_cast<u8*> (memblock), size);

        if (m_bLoaded)
            Log("ROM loaded", path);
        else
            Log("There was a problem loading the memory for file %s...", path);

        SafeDeleteArray(memblock);

        return m_bLoaded;
    }
    else
    {
        Log("There was a problem loading the file %s...", path);
        m_bLoaded = false;
        return m_bLoaded;
    }
}

bool Cartridge::LoadFromBuffer(const u8* buffer, int size)
{
    if (IsValidPointer(buffer))
    {
        m_iTotalSize = size;
        m_pTheROM = new u8[m_iTotalSize];
        memcpy(m_pTheROM, buffer, m_iTotalSize);

        GatherMetadata();

        return true;
    }
    else
        return false;
}

int Cartridge::GetVersion() const
{
    return m_iVersion;
}

bool Cartridge::IsSGB() const
{
    return m_bSGB;
}

bool Cartridge::IsCGB() const
{
    return m_bCGB;
}

void Cartridge::GatherMetadata()
{
    char name[12] = {0};
    name[11] = 0;

    for (int i = 0; i < 11; i++)
    {
        name[i] = m_pTheROM[0x0134 + i];

        if (name[i] == 0)
        {
            break;
        }
    }

    strcpy(m_szName, name);

    m_bCGB = (m_pTheROM[0x143] == 0x80) || (m_pTheROM[0x143] == 0xC0);
    m_bSGB = (m_pTheROM[0x146] == 0x03);
    m_iType = m_pTheROM[0x147];
    m_iROMSize = m_pTheROM[0x148];
    m_iRAMSize = m_pTheROM[0x149];
    m_iVersion = m_pTheROM[0x14C];

    Log("ROM Name %s", m_szName);
    Log("ROM Version %d", m_iVersion);
    Log("ROM Type %X", m_iType);
    Log("ROM Size %X", m_iROMSize);
    Log("RAM Size %X", m_iRAMSize);

    if (m_pTheROM[0x143] == 0xC0)
        Log("Game Boy Color Only!");
    else if (m_bCGB)
        Log("Game Boy Color Supported");

    if (m_bSGB)
        Log("Super Game Boy Supported");

    int checksum = 0;

    for (int j = 0x134; j < 0x14E; j++)
    {
        checksum += m_pTheROM[j];
    }

    m_bValidROM = ((checksum + 25) & BIT_MASK_8) == 0;

    if (m_bValidROM)
        Log("Checksum OK!");
}

