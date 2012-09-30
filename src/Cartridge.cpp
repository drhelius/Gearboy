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

#include <string>
#include <algorithm>
#include "Cartridge.h"
#include "miniz/miniz.c"

Cartridge::Cartridge()
{
    InitPointer(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
    m_RTCCurrentTime = 0;
    m_bBattery = false;
    m_szFilePath[0] = 0;
    m_bRTCPresent = false;
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
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
    m_RTCCurrentTime = 0;
    m_bBattery = false;
    m_szFilePath[0] = 0;
    m_bRTCPresent = false;
}

bool Cartridge::IsValidROM() const
{
    return m_bValidROM;
}

bool Cartridge::IsLoadedROM() const
{
    return m_bLoaded;
}

Cartridge::CartridgeTypes Cartridge::GetType() const
{
    return m_Type;
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

const char* Cartridge::GetFilePath() const
{
    return m_szFilePath;
}

int Cartridge::GetTotalSize() const
{
    return m_iTotalSize;
}

bool Cartridge::HasBattery() const
{
    return m_bBattery;
}

u8* Cartridge::GetTheROM() const
{
    return m_pTheROM;
}

bool Cartridge::LoadFromZipFile(const u8* buffer, int size)
{
    using namespace std;

    mz_zip_archive zip_archive;
    mz_bool status;
    memset(&zip_archive, 0, sizeof (zip_archive));

    status = mz_zip_reader_init_mem(&zip_archive, (void*) buffer, size, 0);
    if (!status)
    {
        Log("mz_zip_reader_init_mem() failed!");
        return false;
    }

    for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++)
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
            Log("mz_zip_reader_file_stat() failed!");
            mz_zip_reader_end(&zip_archive);
            return false;
        }

        Log("ZIP Content - Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u", file_stat.m_filename, file_stat.m_comment, (unsigned int) file_stat.m_uncomp_size, (unsigned int) file_stat.m_comp_size);

        string fn((const char*) file_stat.m_filename);
        transform(fn.begin(), fn.end(), fn.begin(), (int(*)(int)) tolower);
        string extension = fn.substr(fn.find_last_of(".") + 1);

        if ((extension == "gb") || (extension == "dmg") || (extension == "gbc") || (extension == "cgb") || (extension == "sgb"))
        {
            void *p;
            size_t uncomp_size;

            p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncomp_size, 0);
            if (!p)
            {
                Log("mz_zip_reader_extract_file_to_heap() failed!");
                mz_zip_reader_end(&zip_archive);
                return false;
            }

            bool ok = LoadFromBuffer((const u8*) p, uncomp_size);

            free(p);
            mz_zip_reader_end(&zip_archive);

            return ok;
        }
    }
    return false;
}

bool Cartridge::LoadFromFile(const char* path)
{
    using namespace std;

    Log("Loading %s...", path);

    Reset();

    strcpy(m_szFilePath, path);

    ifstream file(path, ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
        int size = static_cast<int> (file.tellg());
        char* memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        string fn(path);
        transform(fn.begin(), fn.end(), fn.begin(), (int(*)(int)) tolower);
        string extension = fn.substr(fn.find_last_of(".") + 1);

        if (extension == "zip")
        {
            Log("Loading from ZIP...");
            m_bLoaded = LoadFromZipFile(reinterpret_cast<u8*> (memblock), size);
        }
        else
        {
            m_bLoaded = LoadFromBuffer(reinterpret_cast<u8*> (memblock), size);
        }

        if (m_bLoaded)
        {
            Log("ROM loaded", path);
        }
        else
        {
            Log("There was a problem loading the memory for file %s...", path);
        }

        SafeDeleteArray(memblock);
    }
    else
    {
        Log("There was a problem loading the file %s...", path);
        m_bLoaded = false;
    }

    if (!m_bLoaded)
    {
        Reset();
    }

    return m_bLoaded;
}

bool Cartridge::LoadFromBuffer(const u8* buffer, int size)
{
    if (IsValidPointer(buffer))
    {
        m_iTotalSize = size;
        m_pTheROM = new u8[m_iTotalSize];
        memcpy(m_pTheROM, buffer, m_iTotalSize);
        return GatherMetadata();
    }
    else
        return false;
}

void Cartridge::CheckCartridgeType(int type)
{
    if ((type != 0xEA) && (GetROMSize() == 0))
        type = 0;

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
            m_Type = CartridgeNoMBC;
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
            // Hack to accept 0xEA as a MBC1 (Sonic 3D Blast 5)
        case 0xFF:
            // Hack to accept HuC1 as a MBC1
            m_Type = CartridgeMBC1;
            break;
        case 0x05:
            // MBC2
        case 0x06:
            // MBC2
            // BATT
            m_Type = CartridgeMBC2;
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
            // Game Boy Camera
            m_Type = CartridgeMBC3;
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
            m_Type = CartridgeMBC5;
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
        case 0x15:
            // MBC4
        case 0x16:
            // MBC4
            // SRAM 
        case 0x17:
            // MBC4
            // SRAM
            // BATT
        case 0x22:
            // MBC7
            // BATT
            // SRAM
        case 0x55:
            // GG
        case 0x56:
            // GS3
        case 0xFD:
            // TAMA 5
        case 0xFE:
            // HuC3
            m_Type = CartridgeNotSupported;
            Log("--> ** This cartridge is not supported. Type: %d", type);
            break;
        default:
            m_Type = CartridgeNotSupported;
            Log("--> ** Unknown cartridge type: %d", type);
    }

    switch (type)
    {
        case 0x03:
        case 0x06:
        case 0x09:
        case 0x0D:
        case 0x0F:
        case 0x10:
        case 0x13:
        case 0x17:
        case 0x1B:
        case 0x1E:
        case 0x22:
        case 0xFD:
        case 0xFF:
            m_bBattery = true;
            break;
        default:
            m_bBattery = false;
    }

    switch (type)
    {
        case 0x0F:
        case 0x10:
            m_bRTCPresent = true;
            break;
        default:
            m_bRTCPresent = false;
    }
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

void Cartridge::UpdateCurrentRTC()
{
    m_RTCCurrentTime = time(NULL);
}

size_t Cartridge::GetCurrentRTC()
{
    return m_RTCCurrentTime;
}

bool Cartridge::IsRTCPresent() const
{
    return m_bRTCPresent;
}

bool Cartridge::GatherMetadata()
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
    int type = m_pTheROM[0x147];
    m_iROMSize = m_pTheROM[0x148];
    m_iRAMSize = m_pTheROM[0x149];
    m_iVersion = m_pTheROM[0x14C];
    
    bool presumeMultiMBC1 = ((type == 1) && (m_iRAMSize == 0) && (m_iROMSize == 5));

    CheckCartridgeType(type);
    
    if ((m_Type == Cartridge::CartridgeMBC1) && presumeMultiMBC1)
    {
        m_Type = Cartridge::CartridgeMBC1Multi;
        Log("Presumed Multi 64");
    }

    Log("ROM Name %s", m_szName);
    Log("ROM Version %d", m_iVersion);
    Log("ROM Type %X", type);
    Log("ROM Size %X", m_iROMSize);
    Log("RAM Size %X", m_iRAMSize);

    switch (m_Type)
    {
        case Cartridge::CartridgeNoMBC:
            Log("No MBC found");
            break;
        case Cartridge::CartridgeMBC1:
            Log("MBC1 found");
            break;
        case Cartridge::CartridgeMBC1Multi:
            Log("MBC1 Multi 64 found");
            break;
        case Cartridge::CartridgeMBC2:
            Log("MBC2 found");
            break;
        case Cartridge::CartridgeMBC3:
            Log("MBC3 found");
            break;
        case Cartridge::CartridgeMBC5:
            Log("MBC5 found");
            break;
        case Cartridge::CartridgeNotSupported:
            Log("Cartridge not supported!!");
            break;
        default:
            break;
    }

    if (m_bBattery)
    {
        Log("Battery powered RAM found");
    }

    if (m_pTheROM[0x143] == 0xC0)
    {
        Log("Game Boy Color only");
    }
    else if (m_bCGB)
    {
        Log("Game Boy Color supported");
    }

    if (m_bSGB)
    {
        Log("Super Game Boy supported");
    }

    int checksum = 0;

    for (int j = 0x134; j < 0x14E; j++)
    {
        checksum += m_pTheROM[j];
    }

    m_bValidROM = ((checksum + 25) & 0xFF) == 0;

    if (m_bValidROM)
    {
        Log("Checksum OK!");
    }
    else
    {
        Log("Checksum FAILED!!!");
    }

    return (m_Type != CartridgeNotSupported);
}

