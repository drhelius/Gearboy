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

#ifndef MBC7MEMORYRULE_H
#define	MBC7MEMORYRULE_H

#include "MemoryRule.h"

class MBC7MemoryRule : public MemoryRule
{
public:
    MBC7MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MBC7MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

    void SetAccelerometer(double x, double y);

private:
    u8 ReadRegister(u16 address);
    void WriteRegister(u16 address, u8 value);
    void ProcessEEPROMCommand();

private:
    int m_iCurrentROMBank;
    int m_CurrentROMAddress;
    bool m_bRamEnable1;
    bool m_bRamEnable2;
    double m_AccelerometerX;
    double m_AccelerometerY;
    u16 m_XLatch;
    u16 m_YLatch;
    bool m_bLatchReady;
    u8 m_EEPROM[256];
    bool m_bEEPROMDoPin;
    bool m_bEEPROMDiPin;
    bool m_bEEPROMClkPin;
    bool m_bEEPROMCsPin;
    u16 m_iEEPROMCommand;
    u16 m_iEEPROMReadBits;
    u8 m_iEEPROMArgumentBitsLeft;
    bool m_bEEPROMWriteEnabled;
};

#endif	/* MBC7MEMORYRULE_H */
