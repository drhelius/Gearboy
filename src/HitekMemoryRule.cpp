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

#include "HitekMemoryRule.h"
#include "Cartridge.h"

static const u8 kDataBitOrder[8][8] =
{
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 5, 6, 3, 4, 2, 1, 7 },
    { 0, 6, 5, 3, 4, 1, 2, 7 },
    { 0, 6, 2, 3, 4, 5, 1, 7 },
    { 0, 5, 2, 3, 4, 6, 1, 7 },
    { 0, 5, 2, 3, 4, 1, 6, 7 },
    { 0, 2, 6, 3, 4, 1, 5, 7 },
    { 0, 2, 6, 3, 4, 5, 1, 7 }
};

static const u8 kBankBitOrder[8][8] =
{
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 1, 2, 3, 7, 6, 5, 4 },
    { 0, 1, 2, 3, 4, 7, 6, 5 },
    { 0, 1, 2, 3, 5, 4, 7, 6 },
    { 0, 1, 2, 3, 6, 5, 4, 7 },
    { 0, 1, 2, 3, 6, 7, 4, 5 },
    { 0, 1, 2, 3, 5, 6, 7, 4 },
    { 0, 1, 2, 3, 6, 4, 7, 5 }
};

HitekMemoryRule::HitekMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5LogoMemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    for (int mode = 0; mode < 8; mode++)
    {
        for (int value = 0; value < 256; value++)
        {
            u8 data = 0;
            u8 bank = 0;
            for (int bit = 0; bit < 8; bit++)
            {
                data |= ((value >> (7 - kDataBitOrder[mode][bit])) & 1) << (7 - bit);
                bank |= ((value >> (7 - kBankBitOrder[mode][bit])) & 1) << (7 - bit);
            }

            m_DataSwap[mode][value] = data;
            m_BankSwap[mode][value] = bank;
        }
    }

    Reset(false);
}

HitekMemoryRule::~HitekMemoryRule()
{
}

u8 HitekMemoryRule::GetMapperType()
{
    return Cartridge::CartridgeHitek;
}

void HitekMemoryRule::Reset(bool bCGB)
{
    MBC5LogoMemoryRule::Reset(bCGB);
    m_DataSwapMode = 7;
    m_BankSwapMode = 7;
    m_iCurrentROMBank = 1;
}

u8 HitekMemoryRule::PerformRead(u16 address)
{
    if ((address >= 0x4000) && (address < 0x8000))
        return m_DataSwap[m_DataSwapMode][m_pCartridge->GetTheROM()[m_iCurrentROMBank * 0x4000 + (address & 0x3FFF)]];

    return MBC5LogoMemoryRule::PerformRead(address);
}

void HitekMemoryRule::PerformWrite(u16 address, u8 value)
{
    if ((address & 0xF000) == 0x3000)
        return;

    if ((address & 0xF000) == 0x2000)
    {
        u8 bank = value;
        switch (address & 0xF0FF)
        {
            case 0x2000:
                bank = m_BankSwap[m_BankSwapMode][value];
                if (bank == 0)
                    bank = 1;
                break;
            case 0x2001:
                m_DataSwapMode = value & 0x07;
                break;
            case 0x2080:
                m_BankSwapMode = value & 0x07;
                break;
        }

        m_iCurrentROMBank = bank & (m_pCartridge->GetROMBankCount() - 1);
        TraceMapperEvent(address, value);
        return;
    }

    MBC5MemoryRule::PerformWrite(address, value);
}

size_t HitekMemoryRule::GetRamSize()
{
    return 0x8000;
}

u8* HitekMemoryRule::GetCurrentRomBank1()
{
    u8* rom = m_pCartridge->GetTheROM() + m_iCurrentROMBank * 0x4000;
    for (int i = 0; i < 0x4000; i++)
        m_ROMView[i] = m_DataSwap[m_DataSwapMode][rom[i]];

    return m_ROMView;
}

int HitekMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void HitekMemoryRule::SaveState(std::ostream& stream)
{
    MBC5LogoMemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_DataSwapMode), sizeof(m_DataSwapMode));
    stream.write(reinterpret_cast<const char*>(&m_BankSwapMode), sizeof(m_BankSwapMode));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
}

void HitekMemoryRule::LoadState(std::istream& stream, u32 version)
{
    MBC5LogoMemoryRule::LoadState(stream, version);
    stream.read(reinterpret_cast<char*>(&m_DataSwapMode), sizeof(m_DataSwapMode));
    stream.read(reinterpret_cast<char*>(&m_BankSwapMode), sizeof(m_BankSwapMode));
    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    m_DataSwapMode &= 0x07;
    m_BankSwapMode &= 0x07;
    m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
}
