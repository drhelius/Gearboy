#include "MBC2MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MBC2MemoryRule::MBC2MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge)
{
    m_iCurrentROMBank = 0;
    m_bRamEnabled = false;
    Reset();
}

u8 MBC2MemoryRule::PerformRead(u16 address)
{
    if (address >= 0x4000 && address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x4000) + (0x4000 * m_iCurrentROMBank)];
    }
    else
        return m_pMemory->Retrieve(address);
}

void MBC2MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x1000)
    {
        m_bRamEnabled = (value & 0x0F) == 0x0A;
    }
    else if (address >= 0x1000 && address < 0x2100)
    {
        Log("--> ** Atempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0x2100 && address < 0x2200)
    {
        m_iCurrentROMBank = value & 0x0F;
    }
    else if (address >= 0x2200 && address < 0x8000)
    {
        Log("--> ** Atempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0xA000 && address < 0xA200)
    {
        if (m_bRamEnabled)
        {
            m_pMemory->Load(address, value & 0x0F);
        }
        else
            Log("--> ** Atempting to write on RAM when ram is disabled %X %X", address, value);
    }
    else if (address >= 0xA200 && address < 0xC000)
    {
        Log("--> ** Atempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0xC000 && address < 0xDE00)
    {
        // Echo of 8K internal RAM
        m_pMemory->Load(address + 0x2000, value);
        m_pMemory->Load(address, value);
    }
    else if (address >= 0xE000 && address < 0xFE00)
    {
        // Echo of 8K internal RAM
        m_pMemory->Load(address - 0x2000, value);
        m_pMemory->Load(address, value);
    }
    else if (address >= 0xFEA0 && address < 0xFF00)
    {
        // Empty area
        Log("--> ** Atempting to write on non usable address %X %X", address, value);
    }
    else
    {
        m_pMemory->Load(address, value);
    }
}

void MBC2MemoryRule::Reset()
{
    m_iCurrentROMBank = 0;
    m_bRamEnabled = false;
}



