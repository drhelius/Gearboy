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

#include <cstring>
#include "MBC6MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

MBC6MemoryRule::MBC6MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo,
        Input* pInput, Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
        pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pPersistentMemory = new u8[kPersistentSize];
    m_pRAMBanks = m_pPersistentMemory;
    m_pFlash = m_pPersistentMemory + kFlashOffset;
    m_pHidden = m_pPersistentMemory + kHiddenOffset;
    Reset(false);
    InitializePersistentMemory();
}

MBC6MemoryRule::~MBC6MemoryRule()
{
    SafeDeleteArray(m_pPersistentMemory);
    InitPointer(m_pRAMBanks);
    InitPointer(m_pFlash);
    InitPointer(m_pHidden);
}

bool MBC6MemoryRule::MapsROMDirectly()
{
    return false;
}

u8 MBC6MemoryRule::GetMapperType()
{
    return Cartridge::CartridgeMBC6;
}

void MBC6MemoryRule::InitializePersistentMemory()
{
    memset(m_pPersistentMemory, 0xFF, kPersistentSize);
    m_pPersistentMemory[kProtectionOffset] = 0;
    m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE, kFlashSize);
}

void MBC6MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBankA = 0;
    m_iCurrentRAMBankB = 1;
    m_iCurrentROMBankA = 2;
    m_iCurrentROMBankB = 3;
    m_bRamEnabled = false;
    m_bFlashEnabled = false;
    m_bFlashWriteEnabled = false;
    m_bWindowAFlash = false;
    m_bWindowBFlash = false;
    ResetFlashMode();
}

u8 MBC6MemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
        return m_pMemory->Retrieve(address);

    if (address < 0x8000)
        return ReadROMWindow(address);

    if (address >= 0xA000 && address < 0xB000)
    {
        if (!m_bRamEnabled)
            return 0xFF;

        int offset = (m_iCurrentRAMBankA * kRAMBankSize) + (address - 0xA000);
        return m_pRAMBanks[offset];
    }

    if (address >= 0xB000 && address < 0xC000)
    {
        if (!m_bRamEnabled)
            return 0xFF;

        int offset = (m_iCurrentRAMBankB * kRAMBankSize) + (address - 0xB000);
        return m_pRAMBanks[offset];
    }

    return m_pMemory->Retrieve(address);
}

u8 MBC6MemoryRule::ReadROMWindow(u16 address)
{
    bool flash = (address < 0x6000) ? m_bWindowAFlash : m_bWindowBFlash;
    if (flash)
    {
        if (!m_bFlashEnabled)
            return 0xFF;
        return ReadFlash(GetFlashAddress(address));
    }

    int bank = (address < 0x6000) ? m_iCurrentROMBankA : m_iCurrentROMBankB;
    int bankCount = GetROMHalfBankCount();
    int bankOffset = (bank % bankCount) * kFlashBankSize;
    int windowOffset = (address < 0x6000) ? (address - 0x4000) : (address - 0x6000);
    int romOffset = bankOffset + windowOffset;

    if (romOffset < 0 || romOffset >= m_pCartridge->GetTotalSize())
        return 0xFF;

    return m_pCartridge->GetTheROM()[romOffset];
}

u8 MBC6MemoryRule::ReadFlash(u32 flashAddress)
{
    flashAddress &= (kFlashSize - 1);

    switch (m_FlashReadMode)
    {
        case FlashReadID:
        {
            switch (flashAddress & 0x03)
            {
                case 0:
                    return 0xC2;
                case 1:
                    return 0x81;
                case 2:
                    return (flashAddress < kFlashSectorSize) ? 0xC2 : 0x00;
                default:
                    return 0xFF;
            }
        }
        case FlashReadHidden:
            return m_pHidden[flashAddress & (kFlashHiddenSize - 1)];
        case FlashProgramMain:
        case FlashProgramHidden:
        case FlashReadStatus:
            return GetFlashStatus();
        case FlashReadArray:
        default:
            return m_pFlash[flashAddress];
    }
}

void MBC6MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x0400)
    {
        bool previous = m_bRamEnabled;
        m_bRamEnabled = ((value & 0x0F) == 0x0A);

        if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            (*m_pRamChangedCallback)();

        if (IsTraceMapperEventEnabled(TRACE_MAPPER_CONTROL))
        {
            LogTraceMapperEvent(address, value, TRACE_MAPPER_CONTROL, m_bRamEnabled ? TRACE_MAPPER_FLAG_RAM_ENABLED : 0, true);
        }
        return;
    }

    if (address < 0x0800)
    {
        m_iCurrentRAMBankA = value & 0x07;
        TraceMapperEvent(address, value, TRACE_MAPPER_RAM_RTC);
        return;
    }

    if (address < 0x0C00)
    {
        m_iCurrentRAMBankB = value & 0x07;
        TraceMapperEvent(address, value, TRACE_MAPPER_RAM_RTC);
        return;
    }

    if (address < 0x1000)
    {
        m_bFlashEnabled = (value & 0x01) != 0;
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
        return;
    }

    if (address == 0x1000)
    {
        m_bFlashWriteEnabled = (value & 0x01) != 0;
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
        return;
    }

    if (address < 0x2000)
        return;

    if (address < 0x2800)
    {
        m_iCurrentROMBankA = value & 0x7F;
        TraceMapperEvent(address, value, TRACE_MAPPER_ROM);
        return;
    }

    if (address < 0x3000)
    {
        m_bWindowAFlash = (value & 0x08) != 0;
        TraceMapperEvent(address, value, TRACE_MAPPER_ROM);
        return;
    }

    if (address < 0x3800)
    {
        m_iCurrentROMBankB = value & 0x7F;
        TraceMapperEvent(address, value, TRACE_MAPPER_ROM);
        return;
    }

    if (address < 0x4000)
    {
        m_bWindowBFlash = (value & 0x08) != 0;
        TraceMapperEvent(address, value, TRACE_MAPPER_ROM);
        return;
    }

    if (address < 0x6000)
    {
        if (m_bWindowAFlash && m_bFlashEnabled)
            HandleFlashWrite(address, GetFlashAddress(address), value);
        return;
    }

    if (address < 0x8000)
    {
        if (m_bWindowBFlash && m_bFlashEnabled)
            HandleFlashWrite(address, GetFlashAddress(address), value);
        return;
    }

    if (address >= 0xA000 && address < 0xB000)
    {
        if (m_bRamEnabled)
        {
            int offset = (m_iCurrentRAMBankA * kRAMBankSize) + (address - 0xA000);
            m_pRAMBanks[offset] = value;
            m_RAMView[address - 0xA000] = value;
        }
        return;
    }

    if (address >= 0xB000 && address < 0xC000)
    {
        if (m_bRamEnabled)
        {
            int offset = (m_iCurrentRAMBankB * kRAMBankSize) + (address - 0xB000);
            m_pRAMBanks[offset] = value;
            m_RAMView[0x1000 + (address - 0xB000)] = value;
        }
        return;
    }

    m_pMemory->Load(address, value);
}

u32 MBC6MemoryRule::GetFlashAddress(u16 address) const
{
    int bank = (address < 0x6000) ? m_iCurrentROMBankA : m_iCurrentROMBankB;
    int offset = (address < 0x6000) ? (address - 0x4000) : (address - 0x6000);
    return (u32)(bank * kFlashBankSize) + (u32)offset;
}

int MBC6MemoryRule::GetROMHalfBankCount() const
{
    int count = (m_pCartridge->GetTotalSize() + kFlashBankSize - 1) / kFlashBankSize;
    return MAX(count, 1);
}

void MBC6MemoryRule::HandleFlashWrite(u16 address, u32 flashAddress, u8 value)
{
    if (value == 0xF0)
    {
        ResetFlashMode();
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
        return;
    }

    if (m_FlashReadMode == FlashProgramMain || m_FlashReadMode == FlashProgramHidden)
    {
        HandleProgramWrite(address, flashAddress, value);
        return;
    }

    if (m_FlashReadMode == FlashReadID || m_FlashReadMode == FlashReadHidden)
        return;

    HandleFlashCommand(address, flashAddress, value);
}

void MBC6MemoryRule::HandleFlashCommand(u16 address, u32 flashAddress, u8 value)
{
    switch (m_FlashCommandState)
    {
        case FlashCommandIdle:
            if (IsFlashAddress5555(flashAddress) && value == 0xAA)
                m_FlashCommandState = FlashCommandUnlock1;
            break;
        case FlashCommandUnlock1:
            if (IsFlashAddress2AAA(flashAddress) && value == 0x55)
                m_FlashCommandState = FlashCommandCommand;
            else
                ResetFlashCommand();
            break;
        case FlashCommandCommand:
            if (!IsFlashAddress5555(flashAddress))
            {
                ResetFlashCommand();
                break;
            }

            switch (value)
            {
                case 0x90:
                    m_FlashReadMode = FlashReadID;
                    ResetFlashCommand();
                    TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
                    break;
                case 0xA0:
                    BeginProgram(FlashProgramMain);
                    ResetFlashCommand();
                    TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
                    break;
                case 0x80:
                    m_FlashCommandPrefix = FlashPrefixErase;
                    m_FlashCommandState = FlashCommandSecondUnlock1;
                    break;
                case 0x60:
                    m_FlashCommandPrefix = FlashPrefixExtended;
                    m_FlashCommandState = FlashCommandSecondUnlock1;
                    break;
                case 0x77:
                    m_FlashCommandPrefix = FlashPrefixHiddenRead;
                    m_FlashCommandState = FlashCommandSecondUnlock1;
                    break;
                default:
                    ResetFlashCommand();
                    break;
            }
            break;
        case FlashCommandSecondUnlock1:
            if (IsFlashAddress5555(flashAddress) && value == 0xAA)
                m_FlashCommandState = FlashCommandSecondUnlock2;
            else
                ResetFlashCommand();
            break;
        case FlashCommandSecondUnlock2:
            if (IsFlashAddress2AAA(flashAddress) && value == 0x55)
                m_FlashCommandState = FlashCommandSecondCommand;
            else
                ResetFlashCommand();
            break;
        case FlashCommandSecondCommand:
            HandleSecondFlashCommand(address, flashAddress, value);
            ResetFlashCommand();
            break;
        default:
            ResetFlashCommand();
            break;
    }
}

void MBC6MemoryRule::HandleSecondFlashCommand(u16 address, u32 flashAddress, u8 value)
{
    bool accepted = false;

    switch (m_FlashCommandPrefix)
    {
        case FlashPrefixErase:
            if (value == 0x10 && IsFlashAddress5555(flashAddress))
            {
                EraseChip();
                EnterStatusMode();
                accepted = true;
            }
            else if (value == 0x30)
            {
                EraseSector(flashAddress);
                EnterStatusMode();
                accepted = true;
            }
            break;
        case FlashPrefixExtended:
            if (value == 0x04 && IsFlashAddress5555(flashAddress) && m_bFlashWriteEnabled)
            {
                EraseHidden();
                EnterStatusMode();
                accepted = true;
            }
            else if (value == 0x20 && flashAddress < kFlashSectorSize && m_bFlashWriteEnabled)
            {
                SetSector0Protected(true);
                EnterStatusMode();
                accepted = true;
            }
            else if (value == 0x40 && flashAddress < kFlashSectorSize && m_bFlashWriteEnabled)
            {
                SetSector0Protected(false);
                EnterStatusMode();
                accepted = true;
            }
            else if (value == 0xE0 && IsFlashAddress5555(flashAddress) && m_bFlashWriteEnabled)
            {
                BeginProgram(FlashProgramHidden);
                accepted = true;
            }
            break;
        case FlashPrefixHiddenRead:
            if (value == 0x77 && IsFlashAddress5555(flashAddress))
            {
                m_FlashReadMode = FlashReadHidden;
                accepted = true;
            }
            break;
        default:
            break;
    }

    if (accepted)
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
}

void MBC6MemoryRule::HandleProgramWrite(u16 address, u32 flashAddress, u8 value)
{
    int position = flashAddress & (kFlashBufferSize - 1);

    if (m_bFlashLastProgramPositionValid && position == m_iFlashLastProgramPosition)
    {
        CommitProgram(flashAddress);
        TraceMapperEvent(address, value, TRACE_MAPPER_CONTROL);
        return;
    }

    m_FlashProgramBuffer[position] = value;
    m_iFlashLastProgramPosition = position;
    m_bFlashLastProgramPositionValid = true;
}

void MBC6MemoryRule::BeginProgram(FlashReadMode mode)
{
    memset(m_FlashProgramBuffer, 0xFF, sizeof(m_FlashProgramBuffer));
    m_iFlashLastProgramPosition = 0;
    m_bFlashLastProgramPositionValid = false;
    m_FlashReadMode = mode;
}

void MBC6MemoryRule::CommitProgram(u32 flashAddress)
{
    bool changed = false;

    if (m_FlashReadMode == FlashProgramMain)
    {
        u32 target = flashAddress & ~(u32)(kFlashBufferSize - 1);
        bool protectedSector = target < kFlashSectorSize && (!m_bFlashWriteEnabled || IsSector0Protected());

        if (!protectedSector && target + kFlashBufferSize <= kFlashSize)
        {
            for (int i = 0; i < kFlashBufferSize; i++)
            {
                u8 programmed = m_pFlash[target + i] & m_FlashProgramBuffer[i];
                if (programmed != m_pFlash[target + i])
                {
                    m_pFlash[target + i] = programmed;
                    changed = true;
                }
            }

            if (changed)
                m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE + target, kFlashBufferSize);
        }
    }
    else if (m_FlashReadMode == FlashProgramHidden && m_bFlashWriteEnabled)
    {
        int target = flashAddress & 0x80;

        for (int i = 0; i < kFlashBufferSize; i++)
        {
            u8 programmed = m_pHidden[target + i] & m_FlashProgramBuffer[i];

            if (programmed != m_pHidden[target + i])
            {
                m_pHidden[target + i] = programmed;
                changed = true;
            }
        }
    }

    if (changed)
        PersistentMemoryChanged();

    EnterStatusMode();
}

void MBC6MemoryRule::EraseChip()
{
    int start = (!m_bFlashWriteEnabled || IsSector0Protected()) ? kFlashSectorSize : 0;

    if (EraseBytes(m_pFlash + start, kFlashSize - start))
    {
        m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE + start, kFlashSize - start);
        PersistentMemoryChanged();
    }
}

void MBC6MemoryRule::EraseSector(u32 flashAddress)
{
    int sector = (flashAddress & (kFlashSize - 1)) / kFlashSectorSize;

    if (sector == 0 && (!m_bFlashWriteEnabled || IsSector0Protected()))
        return;

    int start = sector * kFlashSectorSize;

    if (EraseBytes(m_pFlash + start, kFlashSectorSize))
    {
        m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE + start, kFlashSectorSize);
        PersistentMemoryChanged();
    }
}

void MBC6MemoryRule::EraseHidden()
{
    if (EraseBytes(m_pHidden, kFlashHiddenSize))
        PersistentMemoryChanged();
}

bool MBC6MemoryRule::EraseBytes(u8* memory, int size)
{
    bool changed = false;

    for (int i = 0; i < size; i++)
    {
        if (memory[i] != 0xFF)
        {
            changed = true;
            break;
        }
    }

    if (changed)
        memset(memory, 0xFF, size);

    return changed;
}

void MBC6MemoryRule::SetSector0Protected(bool protect)
{
    bool previous = IsSector0Protected();

    m_pPersistentMemory[kProtectionOffset] = protect ? 1 : 0;

    if (previous != protect)
        PersistentMemoryChanged();
}

bool MBC6MemoryRule::IsSector0Protected() const
{
    return (m_pPersistentMemory[kProtectionOffset] & 0x01) != 0;
}

bool MBC6MemoryRule::IsFlashAddress5555(u32 flashAddress) const
{
    return (flashAddress & 0x7FFF) == 0x5555;
}

bool MBC6MemoryRule::IsFlashAddress2AAA(u32 flashAddress) const
{
    return (flashAddress & 0x7FFF) == 0x2AAA;
}

u8 MBC6MemoryRule::GetFlashStatus() const
{
    return 0x80 | (IsSector0Protected() ? 0x02 : 0x00);
}

void MBC6MemoryRule::ResetFlashCommand()
{
    m_FlashCommandState = FlashCommandIdle;
    m_FlashCommandPrefix = FlashPrefixNone;
}

void MBC6MemoryRule::ResetFlashMode()
{
    ResetFlashCommand();
    m_FlashReadMode = FlashReadArray;
    memset(m_FlashProgramBuffer, 0xFF, sizeof(m_FlashProgramBuffer));
    m_iFlashLastProgramPosition = 0;
    m_bFlashLastProgramPositionValid = false;
}

void MBC6MemoryRule::EnterStatusMode()
{
    m_FlashReadMode = FlashReadStatus;
    m_iFlashLastProgramPosition = 0;
    m_bFlashLastProgramPositionValid = false;
}

void MBC6MemoryRule::PersistentMemoryChanged()
{
    if (IsValidPointer(m_pRamChangedCallback))
        (*m_pRamChangedCallback)();
}

void MBC6MemoryRule::RefreshROMView()
{
    for (int i = 0; i < 0x4000; i++)
        m_ROMView[i] = ReadROMWindow(0x4000 + i);
}

void MBC6MemoryRule::RefreshRAMView()
{
    for (int i = 0; i < 0x1000; i++)
    {
        if (m_bRamEnabled)
        {
            m_RAMView[i] = m_pRAMBanks[(m_iCurrentRAMBankA * kRAMBankSize) + i];
            m_RAMView[0x1000 + i] = m_pRAMBanks[(m_iCurrentRAMBankB * kRAMBankSize) + i];
        }
        else
        {
            m_RAMView[i] = 0xFF;
            m_RAMView[0x1000 + i] = 0xFF;
        }
    }
}

void MBC6MemoryRule::SaveRam(std::ostream& file)
{
    file.write(reinterpret_cast<const char*>(m_pPersistentMemory), kPersistentSize);
}

bool MBC6MemoryRule::LoadRam(std::istream& file, s32 fileSize)
{
    if (fileSize != kFlashSize && fileSize != kHiddenOffset && fileSize != kPersistentSize)
    {
        Log("MBC6MemoryRule incorrect save size. Found: %d", fileSize);
        return false;
    }

    u8* temporary = new u8[kPersistentSize];
    memset(temporary, 0xFF, kPersistentSize);
    temporary[kProtectionOffset] = 0;

    u8* target = temporary;
    if (fileSize == kFlashSize)
        target += kFlashOffset;

    file.read(reinterpret_cast<char*>(target), fileSize);
    bool complete = file.gcount() == fileSize;
    if (complete)
        memcpy(m_pPersistentMemory, temporary, kPersistentSize);

    delete [] temporary;

    if (!complete)
        return false;

    m_pPersistentMemory[kProtectionOffset] &= 0x01;
    m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE, kFlashSize);
    return true;
}

size_t MBC6MemoryRule::GetRamSize()
{
    return kPersistentSize;
}

u8* MBC6MemoryRule::GetRamBanks()
{
    return m_pPersistentMemory;
}

u8* MBC6MemoryRule::GetCurrentRamBank()
{
    RefreshRAMView();
    return m_RAMView;
}

int MBC6MemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBankA;
}

int MBC6MemoryRule::GetCurrentRamBankIndex(u16 address)
{
    return (address < 0xB000) ? m_iCurrentRAMBankA : m_iCurrentRAMBankB;
}

u8* MBC6MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap();
}

int MBC6MemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* MBC6MemoryRule::GetCurrentRomBank1()
{
    RefreshROMView();
    return m_ROMView;
}

int MBC6MemoryRule::GetCurrentRomBank1Index()
{
    return GetCurrentRomBankIndex(0x4000);
}

u16 MBC6MemoryRule::GetCurrentRomBankIndex(u16 address)
{
    if (address < 0x4000)
        return 0;

    int bank = (address < 0x6000) ? m_iCurrentROMBankA : m_iCurrentROMBankB;
    bool flash = (address < 0x6000) ? m_bWindowAFlash : m_bWindowBFlash;

    if (!flash)
        bank %= GetROMHalfBankCount();

    return (u16)(bank | (flash ? 0x80 : 0x00));
}

u32 MBC6MemoryRule::GetPhysicalROMAddress(u16 address)
{
    return GetPhysicalROMAddress(address, GetCurrentRomBankIndex(address));
}

u32 MBC6MemoryRule::GetPhysicalROMAddress(u16 address, u16 bank)
{
    if (address < 0x4000)
        return address;

    u32 offset = (address < 0x6000) ? (address - 0x4000) : (address - 0x6000);
    u32 physical = (u32)(bank & 0x7F) * kFlashBankSize + offset;
    if (bank & 0x80)
        physical += MAX_ROM_SIZE;

    return physical;
}

void MBC6MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_iCurrentRAMBankA), sizeof(m_iCurrentRAMBankA));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentRAMBankB), sizeof(m_iCurrentRAMBankB));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBankA), sizeof(m_iCurrentROMBankA));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBankB), sizeof(m_iCurrentROMBankB));
    stream.write(reinterpret_cast<const char*>(&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*>(&m_bFlashEnabled), sizeof(m_bFlashEnabled));
    stream.write(reinterpret_cast<const char*>(&m_bFlashWriteEnabled), sizeof(m_bFlashWriteEnabled));
    stream.write(reinterpret_cast<const char*>(&m_bWindowAFlash), sizeof(m_bWindowAFlash));
    stream.write(reinterpret_cast<const char*>(&m_bWindowBFlash), sizeof(m_bWindowBFlash));
    stream.write(reinterpret_cast<const char*>(&m_FlashCommandState), sizeof(m_FlashCommandState));
    stream.write(reinterpret_cast<const char*>(&m_FlashCommandPrefix), sizeof(m_FlashCommandPrefix));
    stream.write(reinterpret_cast<const char*>(&m_FlashReadMode), sizeof(m_FlashReadMode));
    stream.write(reinterpret_cast<const char*>(m_FlashProgramBuffer), sizeof(m_FlashProgramBuffer));
    stream.write(reinterpret_cast<const char*>(&m_iFlashLastProgramPosition), sizeof(m_iFlashLastProgramPosition));
    stream.write(reinterpret_cast<const char*>(&m_bFlashLastProgramPositionValid), sizeof(m_bFlashLastProgramPositionValid));
    stream.write(reinterpret_cast<const char*>(m_pPersistentMemory), kPersistentSize);
}

void MBC6MemoryRule::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&m_iCurrentRAMBankA), sizeof(m_iCurrentRAMBankA));
    stream.read(reinterpret_cast<char*>(&m_iCurrentRAMBankB), sizeof(m_iCurrentRAMBankB));
    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBankA), sizeof(m_iCurrentROMBankA));
    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBankB), sizeof(m_iCurrentROMBankB));
    stream.read(reinterpret_cast<char*>(&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*>(&m_bFlashEnabled), sizeof(m_bFlashEnabled));
    stream.read(reinterpret_cast<char*>(&m_bFlashWriteEnabled), sizeof(m_bFlashWriteEnabled));
    stream.read(reinterpret_cast<char*>(&m_bWindowAFlash), sizeof(m_bWindowAFlash));
    stream.read(reinterpret_cast<char*>(&m_bWindowBFlash), sizeof(m_bWindowBFlash));
    stream.read(reinterpret_cast<char*>(&m_FlashCommandState), sizeof(m_FlashCommandState));
    stream.read(reinterpret_cast<char*>(&m_FlashCommandPrefix), sizeof(m_FlashCommandPrefix));
    stream.read(reinterpret_cast<char*>(&m_FlashReadMode), sizeof(m_FlashReadMode));
    stream.read(reinterpret_cast<char*>(m_FlashProgramBuffer), sizeof(m_FlashProgramBuffer));
    stream.read(reinterpret_cast<char*>(&m_iFlashLastProgramPosition), sizeof(m_iFlashLastProgramPosition));
    stream.read(reinterpret_cast<char*>(&m_bFlashLastProgramPositionValid), sizeof(m_bFlashLastProgramPositionValid));
    stream.read(reinterpret_cast<char*>(m_pPersistentMemory), kPersistentSize);

    m_iCurrentRAMBankA &= 0x07;
    m_iCurrentRAMBankB &= 0x07;
    m_iCurrentROMBankA &= 0x7F;
    m_iCurrentROMBankB &= 0x7F;
    m_pPersistentMemory[kProtectionOffset] &= 0x01;
    m_iFlashLastProgramPosition &= (kFlashBufferSize - 1);

    if (m_FlashCommandState < FlashCommandIdle || m_FlashCommandState >= FlashCommandStateCount)
        m_FlashCommandState = FlashCommandIdle;

    if (m_FlashCommandPrefix < FlashPrefixNone || m_FlashCommandPrefix >= FlashPrefixCount)
        m_FlashCommandPrefix = FlashPrefixNone;

    if (m_FlashReadMode < FlashReadArray || m_FlashReadMode >= FlashReadModeCount)
        m_FlashReadMode = FlashReadArray;

    m_pMemory->InvalidateDisassemblerRecords(MAX_ROM_SIZE, kFlashSize);
}
