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

#include "MBC7MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

#ifdef IS_BIG_ENDIAN
static inline u16 LE16(u16 v)
{
    return (v >> 8) | (v << 8);
}
#else
static inline u16 LE16(u16 v)
{
    return v;
}
#endif

MBC7MemoryRule::MBC7MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

MBC7MemoryRule::~MBC7MemoryRule()
{
}

void MBC7MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentROMBank = 1;
    m_CurrentROMAddress = 0x4000;
    m_bRamEnable1 = false;
    m_bRamEnable2 = false;
    m_AccelerometerX = 0.0;
    m_AccelerometerY = 0.0;
    m_XLatch = 0x8000;
    m_YLatch = 0x8000;
    m_bLatchReady = true;
    memset(m_EEPROM, 0xFF, sizeof(m_EEPROM));
    m_bEEPROMDoPin = true;
    m_bEEPROMDiPin = false;
    m_bEEPROMClkPin = false;
    m_bEEPROMCsPin = false;
    m_iEEPROMCommand = 0;
    m_iEEPROMReadBits = 0xFFFF;
    m_iEEPROMArgumentBitsLeft = 0;
    m_bEEPROMWriteEnabled = false;
}

void MBC7MemoryRule::SetAccelerometer(double x, double y)
{
    m_AccelerometerX = x;
    m_AccelerometerY = y;
}

u8 MBC7MemoryRule::ReadRegister(u16 address)
{
    if (address >= 0xB000)
        return 0xFF;

    switch ((address >> 4) & 0x0F)
    {
        case 0x2:
            return m_XLatch & 0xFF;
        case 0x3:
            return (m_XLatch >> 8) & 0xFF;
        case 0x4:
            return m_YLatch & 0xFF;
        case 0x5:
            return (m_YLatch >> 8) & 0xFF;
        case 0x6:
            return 0x00;
        case 0x8:
            return (m_bEEPROMDoPin ? 0x01 : 0x00) |
                   (m_bEEPROMDiPin ? 0x02 : 0x00) |
                   (m_bEEPROMClkPin ? 0x40 : 0x00) |
                   (m_bEEPROMCsPin ? 0x80 : 0x00);
        default:
            return 0xFF;
    }
}

void MBC7MemoryRule::WriteRegister(u16 address, u8 value)
{
    if (address >= 0xB000)
        return;

    switch ((address >> 4) & 0x0F)
    {
        case 0x0:
        {
            if (value == 0x55)
            {
                m_bLatchReady = true;
                m_XLatch = 0x8000;
                m_YLatch = 0x8000;
            }
            break;
        }
        case 0x1:
        {
            if (value == 0xAA && m_bLatchReady)
            {
                m_bLatchReady = false;
                m_XLatch = 0x81D0 + (u16)(0x70 * m_AccelerometerX);
                m_YLatch = 0x81D0 + (u16)(0x70 * m_AccelerometerY);
            }
            break;
        }
        case 0x8:
        {
            m_bEEPROMCsPin = (value & 0x80) != 0;
            m_bEEPROMDiPin = (value & 0x02) != 0;

            if (m_bEEPROMCsPin)
            {
                if (!m_bEEPROMClkPin && (value & 0x40))
                {
                    // Rising edge of clock
                    m_bEEPROMDoPin = (m_iEEPROMReadBits >> 15) & 1;
                    m_iEEPROMReadBits <<= 1;
                    m_iEEPROMReadBits |= 1;

                    if (m_iEEPROMArgumentBitsLeft == 0)
                    {
                        m_iEEPROMCommand <<= 1;
                        m_iEEPROMCommand |= m_bEEPROMDiPin ? 1 : 0;

                        if (m_iEEPROMCommand & 0x400)
                        {
                            ProcessEEPROMCommand();
                        }
                    }
                    else
                    {
                        // Shifting in argument bits for WRITE/WRAL
                        m_iEEPROMArgumentBitsLeft--;
                        m_bEEPROMDoPin = true;

                        if (m_bEEPROMDiPin)
                        {
                            u16 bit = LE16(1 << m_iEEPROMArgumentBitsLeft);
                            if (m_iEEPROMCommand & 0x100)
                            {
                                // WRITE
                                ((u16*)m_EEPROM)[m_iEEPROMCommand & 0x7F] |= bit;
                            }
                            else
                            {
                                // WRAL
                                for (unsigned i = 0; i < 0x7F; i++)
                                {
                                    ((u16*)m_EEPROM)[i] |= bit;
                                }
                            }
                        }

                        if (m_iEEPROMArgumentBitsLeft == 0)
                        {
                            m_iEEPROMCommand = 0;
                            m_iEEPROMReadBits = (m_iEEPROMCommand & 0x100) ? 0xFF : 0x3FFF;
                        }
                    }
                }
            }

            m_bEEPROMClkPin = (value & 0x40) != 0;
            break;
        }
        default:
            break;
    }
}

void MBC7MemoryRule::ProcessEEPROMCommand()
{
    switch ((m_iEEPROMCommand >> 6) & 0x0F)
    {
        case 0x8:
        case 0x9:
        case 0xA:
        case 0xB:
        {
            // READ
            m_iEEPROMReadBits = LE16(((u16*)m_EEPROM)[m_iEEPROMCommand & 0x7F]);
            m_iEEPROMCommand = 0;
            break;
        }
        case 0x3:
        {
            // EWEN (Erase/Write Enable)
            m_bEEPROMWriteEnabled = true;
            m_iEEPROMCommand = 0;
            break;
        }
        case 0x0:
        {
            // EWDS (Erase/Write Disable)
            m_bEEPROMWriteEnabled = false;
            m_iEEPROMCommand = 0;
            break;
        }
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        {
            // WRITE
            if (m_bEEPROMWriteEnabled)
            {
                ((u16*)m_EEPROM)[m_iEEPROMCommand & 0x7F] = 0;
            }
            m_iEEPROMArgumentBitsLeft = 16;
            break;
        }
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
        {
            // ERASE
            if (m_bEEPROMWriteEnabled)
            {
                ((u16*)m_EEPROM)[m_iEEPROMCommand & 0x7F] = 0xFFFF;
                m_iEEPROMReadBits = 0x3FFF;
            }
            m_iEEPROMCommand = 0;
            break;
        }
        case 0x2:
        {
            // ERAL (Erase All)
            if (m_bEEPROMWriteEnabled)
            {
                memset(m_EEPROM, 0xFF, sizeof(m_EEPROM));
                m_iEEPROMReadBits = 0xFF;
            }
            m_iEEPROMCommand = 0;
            break;
        }
        case 0x1:
        {
            // WRAL (Write All)
            if (m_bEEPROMWriteEnabled)
            {
                memset(m_EEPROM, 0, sizeof(m_EEPROM));
            }
            m_iEEPROMArgumentBitsLeft = 16;
            break;
        }
        default:
            break;
    }
}

u8 MBC7MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            if (m_bRamEnable1 && m_bRamEnable2)
                return ReadRegister(address);
            return 0xFF;
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void MBC7MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            m_bRamEnable1 = ((value & 0x0F) == 0x0A);
            break;
        }
        case 0x2000:
        {
            m_iCurrentROMBank = value;
            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            m_bRamEnable2 = (value == 0x40);
            break;
        }
        case 0x6000:
        {
            break;
        }
        case 0xA000:
        {
            if (m_bRamEnable1 && m_bRamEnable2)
                WriteRegister(address, value);
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void MBC7MemoryRule::SaveRam(std::ostream &file)
{
    Debug("MBC7MemoryRule save RAM (EEPROM)...");

    file.write(reinterpret_cast<const char*>(m_EEPROM), sizeof(m_EEPROM));

    Debug("MBC7MemoryRule save RAM done");
}

bool MBC7MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("MBC7MemoryRule load RAM (EEPROM)...");

    if ((fileSize > 0) && (fileSize != sizeof(m_EEPROM)))
    {
        Log("MBC7MemoryRule incorrect EEPROM size. Expected: %d Found: %d", (int)sizeof(m_EEPROM), fileSize);
        return false;
    }

    file.read(reinterpret_cast<char*>(m_EEPROM), sizeof(m_EEPROM));

    Debug("MBC7MemoryRule load RAM done");

    return true;
}

size_t MBC7MemoryRule::GetRamSize()
{
    return sizeof(m_EEPROM);
}

u8* MBC7MemoryRule::GetRamBanks()
{
    return m_EEPROM;
}

u8* MBC7MemoryRule::GetCurrentRamBank()
{
    return m_EEPROM;
}

int MBC7MemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

u8* MBC7MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

int MBC7MemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* MBC7MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int MBC7MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void MBC7MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*>(&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*>(&m_bRamEnable1), sizeof(m_bRamEnable1));
    stream.write(reinterpret_cast<const char*>(&m_bRamEnable2), sizeof(m_bRamEnable2));
    stream.write(reinterpret_cast<const char*>(&m_XLatch), sizeof(m_XLatch));
    stream.write(reinterpret_cast<const char*>(&m_YLatch), sizeof(m_YLatch));
    stream.write(reinterpret_cast<const char*>(&m_bLatchReady), sizeof(m_bLatchReady));
    stream.write(reinterpret_cast<const char*>(m_EEPROM), sizeof(m_EEPROM));
    stream.write(reinterpret_cast<const char*>(&m_bEEPROMDoPin), sizeof(m_bEEPROMDoPin));
    stream.write(reinterpret_cast<const char*>(&m_bEEPROMDiPin), sizeof(m_bEEPROMDiPin));
    stream.write(reinterpret_cast<const char*>(&m_bEEPROMClkPin), sizeof(m_bEEPROMClkPin));
    stream.write(reinterpret_cast<const char*>(&m_bEEPROMCsPin), sizeof(m_bEEPROMCsPin));
    stream.write(reinterpret_cast<const char*>(&m_iEEPROMCommand), sizeof(m_iEEPROMCommand));
    stream.write(reinterpret_cast<const char*>(&m_iEEPROMReadBits), sizeof(m_iEEPROMReadBits));
    stream.write(reinterpret_cast<const char*>(&m_iEEPROMArgumentBitsLeft), sizeof(m_iEEPROMArgumentBitsLeft));
    stream.write(reinterpret_cast<const char*>(&m_bEEPROMWriteEnabled), sizeof(m_bEEPROMWriteEnabled));
}

void MBC7MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*>(&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*>(&m_bRamEnable1), sizeof(m_bRamEnable1));
    stream.read(reinterpret_cast<char*>(&m_bRamEnable2), sizeof(m_bRamEnable2));
    stream.read(reinterpret_cast<char*>(&m_XLatch), sizeof(m_XLatch));
    stream.read(reinterpret_cast<char*>(&m_YLatch), sizeof(m_YLatch));
    stream.read(reinterpret_cast<char*>(&m_bLatchReady), sizeof(m_bLatchReady));
    stream.read(reinterpret_cast<char*>(m_EEPROM), sizeof(m_EEPROM));
    stream.read(reinterpret_cast<char*>(&m_bEEPROMDoPin), sizeof(m_bEEPROMDoPin));
    stream.read(reinterpret_cast<char*>(&m_bEEPROMDiPin), sizeof(m_bEEPROMDiPin));
    stream.read(reinterpret_cast<char*>(&m_bEEPROMClkPin), sizeof(m_bEEPROMClkPin));
    stream.read(reinterpret_cast<char*>(&m_bEEPROMCsPin), sizeof(m_bEEPROMCsPin));
    stream.read(reinterpret_cast<char*>(&m_iEEPROMCommand), sizeof(m_iEEPROMCommand));
    stream.read(reinterpret_cast<char*>(&m_iEEPROMReadBits), sizeof(m_iEEPROMReadBits));
    stream.read(reinterpret_cast<char*>(&m_iEEPROMArgumentBitsLeft), sizeof(m_iEEPROMArgumentBitsLeft));
    stream.read(reinterpret_cast<char*>(&m_bEEPROMWriteEnabled), sizeof(m_bEEPROMWriteEnabled));
}
