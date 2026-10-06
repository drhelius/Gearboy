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

#include "GGB81MemoryRule.h"
#include "Cartridge.h"

static const u8 kDataBitOrder[8][8] =
{
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 2, 1, 3, 4, 6, 5, 7 },
    { 0, 6, 5, 3, 4, 2, 1, 7 },
    { 0, 1, 5, 3, 4, 6, 2, 7 },
    { 0, 1, 6, 3, 4, 5, 2, 7 },
    { 0, 6, 2, 3, 4, 1, 5, 7 },
    { 0, 2, 5, 3, 4, 1, 6, 7 },
    { 0, 6, 1, 3, 4, 2, 5, 7 }
};

GGB81MemoryRule::GGB81MemoryRule(Memory* pMemory, Cartridge* pCartridge) :
        MBC5LogoMemoryRule(pMemory, pCartridge)
{
    for (int mode = 0; mode < 8; mode++)
    {
        for (int value = 0; value < 256; value++)
        {
            u8 result = 0;
            for (int bit = 0; bit < 8; bit++)
                result |= ((value >> (7 - kDataBitOrder[mode][bit])) & 1) << (7 - bit);

            m_DataSwap[mode][value] = result;
        }
    }

    Reset(false);
}

GGB81MemoryRule::~GGB81MemoryRule()
{
}

u8 GGB81MemoryRule::GetMapperType()
{
    return Cartridge::CartridgeGGB81;
}

void GGB81MemoryRule::Reset(bool bCGB)
{
    MBC5LogoMemoryRule::Reset(bCGB);
    m_DataSwapMode = 0;
}

u8 GGB81MemoryRule::PerformRead(u16 address)
{
    if ((address >= 0x4000) && (address < 0x8000))
        return m_DataSwap[m_DataSwapMode][MBC5MemoryRule::PerformRead(address)];

    return MBC5LogoMemoryRule::PerformRead(address);
}

void GGB81MemoryRule::PerformWrite(u16 address, u8 value)
{
    if ((address & 0xF0FF) == 0x2001)
        m_DataSwapMode = value & 0x07;

    MBC5MemoryRule::PerformWrite(address, value);
}

size_t GGB81MemoryRule::GetRamSize()
{
    return 0x8000;
}

u8* GGB81MemoryRule::GetCurrentRomBank1()
{
    u8* rom = MBC5MemoryRule::GetCurrentRomBank1();
    for (int i = 0; i < 0x4000; i++)
        m_ROMView[i] = m_DataSwap[m_DataSwapMode][rom[i]];

    return m_ROMView;
}

void GGB81MemoryRule::SaveState(std::ostream& stream)
{
    MBC5LogoMemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_DataSwapMode), sizeof(m_DataSwapMode));
}

void GGB81MemoryRule::LoadState(std::istream& stream, u32 version)
{
    MBC5LogoMemoryRule::LoadState(stream, version);
    stream.read(reinterpret_cast<char*>(&m_DataSwapMode), sizeof(m_DataSwapMode));
    m_DataSwapMode &= 0x07;
}
