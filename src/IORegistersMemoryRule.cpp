#include "IORegistersMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"

IORegistersMemoryRule::IORegistersMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo) : MemoryRule(pProcessor, pMemory, pVideo)
{
}

u8 IORegistersMemoryRule::PerformRead(u16 address)
{
    return m_pMemory->Retrieve(address);
}

void IORegistersMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address == 0xFF04)
    {
        // DIV
        m_pMemory->Load(address, 0x00);
    }
    else if (address == 0xFF07)
    {
        // TAC
        value &= 0x07;
        u8 current_tac = m_pMemory->Retrieve(0xFF07);
        if ((current_tac & 0x03) != (value & 0x03))
        {
            m_pProcessor->ResetTIMACycles();
        }
        m_pMemory->Load(address, value);
    }
    else if (address == 0xFF46)
    {
        // DMA
        m_pMemory->DoDMATransfer(value);
    }
    else
    {
        m_pMemory->Load(address, value);
    }
}

