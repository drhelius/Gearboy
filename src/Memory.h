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

#ifndef MEMORY_H
#define	MEMORY_H

#include "definitions.h"
#include "MemoryRule.h"
#include <vector>

class Processor;
class Video;
class CommonMemoryRule;
class IORegistersMemoryRule;

class Memory
{
public:
    struct stDisassembleRecord
    {
        u16 address;
        char name[32];
        char bytes[16];
        int size;
        int bank;
        u8 opcodes[4];
        bool jump;
        u16 jump_address;
    };

    struct stMemoryBreakpoint
    {
        u16 address;
        bool read;
        bool write;
    };

public:
    Memory();
    ~Memory();
    void SetProcessor(Processor* pProcessor);
    void SetVideo(Video* pVideo);
    void Init();
    void Reset(bool bCGB);
    void SetCurrentRule(MemoryRule* pRule);
    void SetCommonRule(CommonMemoryRule* pRule);
    void SetIORule(IORegistersMemoryRule* pRule);
    MemoryRule* GetCurrentRule();
    u8* GetMemoryMap();
    u8 Read(u16 address);
    void Write(u16 address, u8 value);
    u8 ReadCGBWRAM(u16 address);
    void WriteCGBWRAM(u16 address, u8 value);
    void SwitchCGBWRAM(u8 value);
    u8 ReadCGBLCDRAM(u16 address, bool forceBank1);
    void WriteCGBLCDRAM(u16 address, u8 value);
    void SwitchCGBLCDRAM(u8 value);
    u8 Retrieve(u16 address);
    void Load(u16 address, u8 value);
    stDisassembleRecord** GetDisassembledMemoryMap();
    stDisassembleRecord** GetDisassembledROMMemoryMap();
    void LoadBank0and1FromROM(u8* pTheROM);
    void MemoryDump(const char* szFilePath);
    void PerformDMA(u8 value);
    void SwitchCGBDMA(u8 value);
    unsigned int PerformHDMA();
    void PerformGDMA(u8 value);
    bool IsHDMAEnabled() const;
    void SetHDMARegister(int reg, u8 value);
    u8 GetHDMARegister(int reg) const;
    u8* GetCGBRAM();
    int GetCurrentCGBRAMBank();
    int GetCurrentLCDRAMBank();
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
    u8* GetROM0();
    u8* GetROM1();
    u8* GetVRAM();
    u8* GetRAM();
    u8* GetWRAM0();
    u8* GetWRAM1();
    std::vector<stDisassembleRecord*>* GetBreakpointsCPU();
    std::vector<stMemoryBreakpoint>* GetBreakpointsMem();
    stDisassembleRecord* GetRunToBreakpoint();
    void SetRunToBreakpoint(stDisassembleRecord* pBreakpoint);
    void EnableBootromDMG(bool enable);
    void EnableBootromGBC(bool enable);
    void LoadBootromDMG(const char* szFilePath);
    void LoadBootromGBC(const char* szFilePath);

private:
    void LoadBootroom(const char* szFilePath, bool gbc);

private:
    Processor* m_pProcessor;
    Video* m_pVideo;
    CommonMemoryRule* m_pCommonMemoryRule;
    IORegistersMemoryRule* m_pIORegistersMemoryRule;
    MemoryRule* m_pCurrentMemoryRule;
    u8* m_pMap;
    stDisassembleRecord** m_pDisassembledMap;
    stDisassembleRecord** m_pDisassembledROMMap;
    std::vector<stDisassembleRecord*> m_BreakpointsCPU;
    std::vector<stMemoryBreakpoint> m_BreakpointsMem;
    stDisassembleRecord* m_pRunToBreakpoint;
    bool m_bCGB;
    int m_iCurrentWRAMBank;
    int m_iCurrentLCDRAMBank;
    u8* m_pWRAMBanks;
    u8* m_pLCDRAMBank1;
    bool m_bHDMAEnabled;
    int m_iHDMABytes;
    u8 m_HDMA[5];
    u16 m_HDMASource;
    u16 m_HDMADestination;
    bool m_bBootromDMG;
    bool m_bBootromGBC;
    u8* m_pBootromDMG;
    u8* m_pBootromGBC;
};

#include "Memory_inline.h"

// From Gambatte emulator
const u8 kInitialValuesForFFXX[256] = {
    0xCF, 0x00, 0x7E, 0xFF, 0xD3, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA, 0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
    0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
    0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
    0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
    0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
    0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
    0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
    0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00
};

const u8 kInitialValuesForColorFFXX[256] = {
    0xCF, 0x00, 0x7C, 0xFF, 0x44, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
    0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
    0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
    0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
    0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
    0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
};

#endif	/* MEMORY_H */
