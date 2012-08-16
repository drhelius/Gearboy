#include "RomOnlyMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

RomOnlyMemoryRule::RomOnlyMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge)
{
}

u8 RomOnlyMemoryRule::PerformRead(u16 address)
{
    if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
            return m_pMemory->Retrieve(address);
        else
        {
            Log("--> ** Attempting to read from non usable address %X", address);
            return 0x00;
        }
    }
    else if (address >= 0xFEA0 && address < 0xFF00)
    {
        // Empty area
        Log("--> ** Attempting to read from non usable address %X", address);
        return 0x00;
    }
    else
        return m_pMemory->Retrieve(address);
}

void RomOnlyMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        // ROM
        Log("--> ** Attempting to write on ROM address %X %X", address, value);
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
        {
            m_pMemory->Load(address, value);
        }
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
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else
    {
        m_pMemory->Load(address, value);
    }
}

void RomOnlyMemoryRule::Reset()
{
}

