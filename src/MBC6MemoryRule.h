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

#ifndef MBC6MEMORYRULE_H
#define MBC6MEMORYRULE_H

#include "MemoryRule.h"

class MBC6MemoryRule : public MemoryRule
{
public:
    MBC6MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput,
            Cartridge* pCartridge, Audio* pAudio);
    virtual ~MBC6MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual bool MapsROMDirectly();
    virtual u8 GetMapperType();
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream& file);
    virtual bool LoadRam(std::istream& file, s32 fileSize);
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual int GetCurrentRamBankIndex(u16 address);
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual u16 GetCurrentRomBankIndex(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address);
    virtual u32 GetPhysicalROMAddress(u16 address, u16 bank);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

    void InitializePersistentMemory();

private:
    enum FlashCommandState
    {
        FlashCommandIdle = 0,
        FlashCommandUnlock1,
        FlashCommandCommand,
        FlashCommandSecondUnlock1,
        FlashCommandSecondUnlock2,
        FlashCommandSecondCommand,
        FlashCommandStateCount
    };

    enum FlashCommandPrefix
    {
        FlashPrefixNone = 0,
        FlashPrefixErase,
        FlashPrefixExtended,
        FlashPrefixHiddenRead,
        FlashPrefixCount
    };

    enum FlashReadMode
    {
        FlashReadArray = 0,
        FlashReadID,
        FlashReadHidden,
        FlashProgramMain,
        FlashProgramHidden,
        FlashReadStatus,
        FlashReadModeCount
    };

    static const int kRAMSize = 0x8000;
    static const int kRAMBankSize = 0x1000;
    static const int kFlashSize = 0x100000;
    static const int kFlashBankSize = 0x2000;
    static const int kFlashSectorSize = 0x20000;
    static const int kFlashHiddenSize = 0x100;
    static const int kFlashBufferSize = 0x80;
    static const int kFlashOffset = kRAMSize;
    static const int kHiddenOffset = kFlashOffset + kFlashSize;
    static const int kProtectionOffset = kHiddenOffset + kFlashHiddenSize;
    static const int kPersistentSize = kProtectionOffset + 1;

private:
    u8 ReadROMWindow(u16 address);
    u8 ReadFlash(u32 flashAddress);
    u32 GetFlashAddress(u16 address) const;
    int GetROMHalfBankCount() const;
    void HandleFlashWrite(u16 address, u32 flashAddress, u8 value);
    void HandleFlashCommand(u16 address, u32 flashAddress, u8 value);
    void HandleSecondFlashCommand(u16 address, u32 flashAddress, u8 value);
    void HandleProgramWrite(u16 address, u32 flashAddress, u8 value);
    void BeginProgram(FlashReadMode mode);
    void CommitProgram(u32 flashAddress);
    void EraseChip();
    void EraseSector(u32 flashAddress);
    void EraseHidden();
    bool EraseBytes(u8* memory, int size);
    void SetSector0Protected(bool protect);
    bool IsSector0Protected() const;
    bool IsFlashAddress5555(u32 flashAddress) const;
    bool IsFlashAddress2AAA(u32 flashAddress) const;
    u8 GetFlashStatus() const;
    void ResetFlashCommand();
    void ResetFlashMode();
    void EnterStatusMode();
    void PersistentMemoryChanged();
    void RefreshROMView();
    void RefreshRAMView();

private:
    int m_iCurrentRAMBankA;
    int m_iCurrentRAMBankB;
    int m_iCurrentROMBankA;
    int m_iCurrentROMBankB;
    bool m_bRamEnabled;
    bool m_bFlashEnabled;
    bool m_bFlashWriteEnabled;
    bool m_bWindowAFlash;
    bool m_bWindowBFlash;
    FlashCommandState m_FlashCommandState;
    FlashCommandPrefix m_FlashCommandPrefix;
    FlashReadMode m_FlashReadMode;
    u8 m_FlashProgramBuffer[kFlashBufferSize];
    int m_iFlashLastProgramPosition;
    bool m_bFlashLastProgramPositionValid;
    u8* m_pPersistentMemory;
    u8* m_pRAMBanks;
    u8* m_pFlash;
    u8* m_pHidden;
    u8 m_ROMView[0x4000];
    u8 m_RAMView[0x2000];
};

#endif /* MBC6MEMORYRULE_H */
