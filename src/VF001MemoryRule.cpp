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

#include "VF001MemoryRule.h"
#include "Cartridge.h"
#include <cstring>

VF001MemoryRule::VF001MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5LogoMemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_InitialValue = 0;
    Reset(false);
}

VF001MemoryRule::~VF001MemoryRule()
{
}

u8 VF001MemoryRule::GetMapperType()
{
    return m_InitialValue == 0x10 ? Cartridge::CartridgeVF001A : Cartridge::CartridgeVF001;
}

void VF001MemoryRule::SetInitialValue(u8 value)
{
    m_InitialValue = value;
}

void VF001MemoryRule::Reset(bool bCGB)
{
    MBC5LogoMemoryRule::Reset(bCGB);
    m_bConfigMode = false;
    m_RunningValue = 0;
    memset(m_Registers, 0, sizeof(m_Registers));
    m_ReplacementBankLatch = 0;
    m_SequenceBank = 0;
    m_SequenceAddress = 0;
    m_SequenceLength = 0;
    memset(m_Sequence, 0, sizeof(m_Sequence));
    m_SequenceRemaining = 0;
    m_bReplace = false;
    m_ReplacementAddress = 0;
    m_ReplacementBank = 0;
}

u8 VF001MemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
        return ReadROM(address, false);

    return MBC5MemoryRule::PerformRead(address);
}

u8 VF001MemoryRule::DebugRead(u16 address)
{
    if (address < 0x8000)
        return ReadROM(address, true);

    return MBC5MemoryRule::PerformRead(address);
}

u8 VF001MemoryRule::ReadROM(u16 address, bool debug)
{
    u8 remaining = m_SequenceRemaining;
    int bank = address < 0x4000 ? 0 : GetCurrentRomBank1Index();

    if ((remaining == 0) && (bank == m_SequenceBank) && (address == m_SequenceAddress))
        remaining = m_SequenceLength;

    if (remaining > 0)
    {
        u8 value = m_Sequence[m_SequenceLength - remaining];
        if (!debug)
            m_SequenceRemaining = remaining - 1;
        return value;
    }

    if (m_bReplace && (address >= m_ReplacementAddress) && (address < 0x4000))
    {
        int replacement = m_ReplacementBank & (m_pCartridge->GetROMBankCount() - 1);
        return m_pCartridge->GetTheROM()[replacement * 0x4000 + address];
    }

    return debug ? MBC5LogoMemoryRule::DebugRead(address) : MBC5LogoMemoryRule::PerformRead(address);
}

void VF001MemoryRule::PerformWrite(u16 address, u8 value)
{
    if ((address < 0x6000) || (address >= 0x8000))
    {
        MBC5MemoryRule::PerformWrite(address, value);
        return;
    }

    u16 reg = address & 0xF00F;

    if ((reg == 0x7000) && (value == 0x96))
    {
        m_bConfigMode = true;
        m_RunningValue = m_InitialValue;
    }
    else if ((reg == 0x700F) && (value == 0x96))
        m_bConfigMode = false;
    else
    {
        if (!m_bConfigMode || ((reg != 0x6000) && ((reg < 0x7000) || (reg > 0x700A))))
            return;

        m_RunningValue = ((m_RunningValue >> 1) | (m_RunningValue << 7)) ^ value;

        if (reg == 0x6000)
            m_ReplacementBankLatch = m_RunningValue;
        else
            m_Registers[reg & 0x0F] = m_RunningValue;

        if (reg == 0x7000)
        {
            m_SequenceAddress = (m_Registers[2] << 8) | m_Registers[1];
            m_SequenceBank = m_Registers[3];
            memcpy(m_Sequence, m_Registers + 4, sizeof(m_Sequence));
            u8 command = m_Registers[0] & 0x07;
            m_SequenceLength = command >= 4 ? command - 3 : 0;
            m_SequenceRemaining = 0;
        }
        else if (reg == 0x7008)
        {
            m_ReplacementAddress = (m_Registers[10] << 8) | m_Registers[9];
            m_ReplacementBank = m_ReplacementBankLatch;
            m_bReplace = (m_Registers[8] & 0x0F) == 0x0F;
        }
    }

    TraceMapperEvent(address, value);
}

size_t VF001MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMSize() > 0 ? 0x2000 : 0;
}

u8* VF001MemoryRule::GetRomBank0()
{
    memcpy(m_ROM0View, MBC5MemoryRule::GetRomBank0(), sizeof(m_ROM0View));
    if (m_bReplace && (m_ReplacementAddress < 0x4000))
    {
        int bank = m_ReplacementBank & (m_pCartridge->GetROMBankCount() - 1);
        memcpy(m_ROM0View + m_ReplacementAddress,
            m_pCartridge->GetTheROM() + bank * 0x4000 + m_ReplacementAddress,
            0x4000 - m_ReplacementAddress);
    }
    return m_ROM0View;
}

u16 VF001MemoryRule::GetCurrentRomBankIndex(u16 address)
{
    if (m_bReplace && (address >= m_ReplacementAddress) && (address < 0x4000))
        return m_ReplacementBank & (m_pCartridge->GetROMBankCount() - 1);

    return MBC5MemoryRule::GetCurrentRomBankIndex(address);
}

void VF001MemoryRule::SaveState(std::ostream& stream)
{
    MBC5LogoMemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_bConfigMode), sizeof(m_bConfigMode));
    stream.write(reinterpret_cast<const char*>(&m_InitialValue), sizeof(m_InitialValue));
    stream.write(reinterpret_cast<const char*>(&m_RunningValue), sizeof(m_RunningValue));
    stream.write(reinterpret_cast<const char*>(m_Registers), sizeof(m_Registers));
    stream.write(reinterpret_cast<const char*>(&m_ReplacementBankLatch), sizeof(m_ReplacementBankLatch));
    stream.write(reinterpret_cast<const char*>(&m_SequenceBank), sizeof(m_SequenceBank));
    stream.write(reinterpret_cast<const char*>(&m_SequenceAddress), sizeof(m_SequenceAddress));
    stream.write(reinterpret_cast<const char*>(&m_SequenceLength), sizeof(m_SequenceLength));
    stream.write(reinterpret_cast<const char*>(m_Sequence), sizeof(m_Sequence));
    stream.write(reinterpret_cast<const char*>(&m_SequenceRemaining), sizeof(m_SequenceRemaining));
    stream.write(reinterpret_cast<const char*>(&m_bReplace), sizeof(m_bReplace));
    stream.write(reinterpret_cast<const char*>(&m_ReplacementAddress), sizeof(m_ReplacementAddress));
    stream.write(reinterpret_cast<const char*>(&m_ReplacementBank), sizeof(m_ReplacementBank));
}

void VF001MemoryRule::LoadState(std::istream& stream, u32 version)
{
    MBC5LogoMemoryRule::LoadState(stream, version);
    stream.read(reinterpret_cast<char*>(&m_bConfigMode), sizeof(m_bConfigMode));
    stream.read(reinterpret_cast<char*>(&m_InitialValue), sizeof(m_InitialValue));
    stream.read(reinterpret_cast<char*>(&m_RunningValue), sizeof(m_RunningValue));
    stream.read(reinterpret_cast<char*>(m_Registers), sizeof(m_Registers));
    stream.read(reinterpret_cast<char*>(&m_ReplacementBankLatch), sizeof(m_ReplacementBankLatch));
    stream.read(reinterpret_cast<char*>(&m_SequenceBank), sizeof(m_SequenceBank));
    stream.read(reinterpret_cast<char*>(&m_SequenceAddress), sizeof(m_SequenceAddress));
    stream.read(reinterpret_cast<char*>(&m_SequenceLength), sizeof(m_SequenceLength));
    stream.read(reinterpret_cast<char*>(m_Sequence), sizeof(m_Sequence));
    stream.read(reinterpret_cast<char*>(&m_SequenceRemaining), sizeof(m_SequenceRemaining));
    stream.read(reinterpret_cast<char*>(&m_bReplace), sizeof(m_bReplace));
    stream.read(reinterpret_cast<char*>(&m_ReplacementAddress), sizeof(m_ReplacementAddress));
    stream.read(reinterpret_cast<char*>(&m_ReplacementBank), sizeof(m_ReplacementBank));
    if (m_SequenceLength > 4)
        m_SequenceLength = 0;
    if (m_SequenceRemaining > m_SequenceLength)
        m_SequenceRemaining = 0;
}
