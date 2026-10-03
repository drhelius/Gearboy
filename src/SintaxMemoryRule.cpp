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

#include "SintaxMemoryRule.h"
#include "Cartridge.h"

static const u8 kBankBitOrder[16][8] =
{
    { 0, 7, 2, 1, 4, 3, 6, 5 },
    { 7, 6, 1, 0, 3, 2, 5, 4 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 1, 6, 7, 4, 5, 2, 3 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 5, 7, 4, 6, 2, 3, 0, 1 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 3, 2, 5, 4, 7, 6, 1, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 5, 4, 7, 6, 1, 0, 3, 2 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 6, 7, 0, 1, 2, 3, 4, 5 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 1, 2, 3, 4, 5, 6, 7 }
};

SintaxMemoryRule::SintaxMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5LogoMemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    for (int mode = 0; mode < 16; mode++)
    {
        for (int value = 0; value < 256; value++)
        {
            u8 bank = 0;
            for (int bit = 0; bit < 8; bit++)
                bank |= ((value >> (7 - kBankBitOrder[mode][bit])) & 1) << (7 - bit);

            m_BankSwap[mode][value] = bank;
        }
    }

    Reset(false);
}

SintaxMemoryRule::~SintaxMemoryRule()
{
}

u8 SintaxMemoryRule::GetMapperType()
{
    return Cartridge::CartridgeSintax;
}

void SintaxMemoryRule::Reset(bool bCGB)
{
    MBC5LogoMemoryRule::Reset(bCGB);
    m_BankSwapMode = 0;
    m_BankSelect = 1;
    m_RomBank = 1;
    memset(m_Xor, 0, sizeof(m_Xor));
    m_iCurrentROMBank = 1;
}

u8 SintaxMemoryRule::PerformRead(u16 address)
{
    if ((address >= 0x4000) && (address < 0x8000))
        return m_pCartridge->GetTheROM()[m_iCurrentROMBank * 0x4000 + (address & 0x3FFF)] ^ m_Xor[m_BankSelect & 0x03];

    return MBC5LogoMemoryRule::PerformRead(address);
}

void SintaxMemoryRule::PerformWrite(u16 address, u8 value)
{
    if ((address & 0xF000) == 0x2000)
    {
        m_BankSelect = value;
        m_RomBank = (m_RomBank & 0x100) | m_BankSwap[m_BankSwapMode][value];
        UpdateBank();
        TraceMapperEvent(address, value);
        return;
    }

    if ((address & 0xF000) == 0x3000)
    {
        m_RomBank = (m_RomBank & 0xFF) | ((value & 0x01) << 8);
        UpdateBank();
        TraceMapperEvent(address, value);
        return;
    }

    if ((address & 0xF0F0) == 0x5010)
    {
        m_BankSwapMode = value & 0x0F;
        m_RomBank = (m_RomBank & 0x100) | m_BankSwap[m_BankSwapMode][m_BankSelect];
        UpdateBank();
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
        return;
    }

    if ((address & 0xF000) == 0x7000)
    {
        int index = (address >> 4) & 0x0F;
        if ((index >= 2) && (index <= 5))
        {
            m_Xor[index - 2] = value;
            TraceMapperEvent(address, value);
        }
        return;
    }

    MBC5MemoryRule::PerformWrite(address, value);
}

void SintaxMemoryRule::UpdateBank()
{
    m_iCurrentROMBank = m_RomBank & (m_pCartridge->GetROMBankCount() - 1);
}

size_t SintaxMemoryRule::GetRamSize()
{
    return 0x8000;
}

u8* SintaxMemoryRule::GetCurrentRomBank1()
{
    u8* rom = m_pCartridge->GetTheROM() + m_iCurrentROMBank * 0x4000;
    u8 mask = m_Xor[m_BankSelect & 0x03];
    for (int i = 0; i < 0x4000; i++)
        m_ROMView[i] = rom[i] ^ mask;

    return m_ROMView;
}

int SintaxMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void SintaxMemoryRule::SaveState(std::ostream& stream)
{
    MBC5LogoMemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_BankSwapMode), sizeof(m_BankSwapMode));
    stream.write(reinterpret_cast<const char*>(&m_BankSelect), sizeof(m_BankSelect));
    stream.write(reinterpret_cast<const char*>(&m_RomBank), sizeof(m_RomBank));
    stream.write(reinterpret_cast<const char*>(m_Xor), sizeof(m_Xor));
}

void SintaxMemoryRule::LoadState(std::istream& stream, u32 version)
{
    MBC5LogoMemoryRule::LoadState(stream, version);
    stream.read(reinterpret_cast<char*>(&m_BankSwapMode), sizeof(m_BankSwapMode));
    stream.read(reinterpret_cast<char*>(&m_BankSelect), sizeof(m_BankSelect));
    stream.read(reinterpret_cast<char*>(&m_RomBank), sizeof(m_RomBank));
    stream.read(reinterpret_cast<char*>(m_Xor), sizeof(m_Xor));
    m_BankSwapMode &= 0x0F;
    m_RomBank &= 0x1FF;
    UpdateBank();
}
