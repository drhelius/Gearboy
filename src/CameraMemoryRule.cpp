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

#include "CameraMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

const int kCameraRamBanksSize = 0x20000;

CameraMemoryRule::CameraMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[kCameraRamBanksSize];
    Reset(false);
}

CameraMemoryRule::~CameraMemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void CameraMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
    m_bCameraRegistersSelected = false;
    for (int i = 0; i < kCameraRamBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    memset(m_CameraRegisters, 0, sizeof(m_CameraRegisters));
    m_bCapturing = false;
    m_iCaptureClocks = 0;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
}

u8 CameraMemoryRule::PerformRead(u16 address)
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
            if (m_bCameraRegistersSelected)
            {
                if (address == 0xA000)
                {
                    // Register A000: bit 0 = capture in progress
                    return m_bCapturing ? 0x01 : 0x00;
                }
                // Other camera registers return 0x00
                return 0x00;
            }

            if (m_bRamEnabled)
            {
                if (m_bCapturing)
                    return 0x00;
                return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
            }
            else
            {
                Debug("--> ** Attempting to read from disabled ram %X", address);
                return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void CameraMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            bool previous = m_bRamEnabled;
            m_bRamEnabled = ((value & 0x0F) == 0x0A);

            if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            {
                (*m_pRamChangedCallback)();
            }
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
            m_iCurrentRAMBank = value & 0x0F;
            m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
            m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
            m_bCameraRegistersSelected = (value & 0x10) != 0;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            Debug("--> ** Attempting to write on invalid address %X %X", address, value);
            break;
        }
        case 0xA000:
        {
            if (m_bCameraRegistersSelected)
            {
                int reg = address & 0x7F;
                if (reg < 0x36)
                {
                    m_CameraRegisters[reg] = value;
                }

                if (reg == 0x00 && (value & 0x01))
                {
                    // TODO: Start capture immediately finish with blank image
                    m_bCapturing = false;
                }
            }
            else if (m_bRamEnabled)
            {
                if (!m_bCapturing)
                    m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
            }
            else
            {
                Debug("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
            }
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void CameraMemoryRule::SaveRam(std::ostream &file)
{
    Debug("CameraMemoryRule save RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("CameraMemoryRule save RAM done");
}

bool CameraMemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("CameraMemoryRule load RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    if ((fileSize > 0) && (fileSize != ramSize))
    {
        Log("CameraMemoryRule incorrect size. Expected: %d Found: %d", ramSize, fileSize);
        return false;
    }

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Debug("CameraMemoryRule load RAM done");

    return true;
}

size_t CameraMemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

u8* CameraMemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* CameraMemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int CameraMemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank;
}

u8* CameraMemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

int CameraMemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* CameraMemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int CameraMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void CameraMemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (&m_bCameraRegistersSelected), sizeof(m_bCameraRegistersSelected));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), kCameraRamBanksSize);
    stream.write(reinterpret_cast<const char*> (m_CameraRegisters), sizeof(m_CameraRegisters));
    stream.write(reinterpret_cast<const char*> (&m_bCapturing), sizeof(m_bCapturing));
    stream.write(reinterpret_cast<const char*> (&m_iCaptureClocks), sizeof(m_iCaptureClocks));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

void CameraMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (&m_bCameraRegistersSelected), sizeof(m_bCameraRegistersSelected));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), kCameraRamBanksSize);
    stream.read(reinterpret_cast<char*> (m_CameraRegisters), sizeof(m_CameraRegisters));
    stream.read(reinterpret_cast<char*> (&m_bCapturing), sizeof(m_bCapturing));
    stream.read(reinterpret_cast<char*> (&m_iCaptureClocks), sizeof(m_iCaptureClocks));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}
