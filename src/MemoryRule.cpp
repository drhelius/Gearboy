#include "MemoryRule.h"

MemoryRule::MemoryRule(Processor* pProcessor, Memory* pMemory,
        Video* pVideo, Input* pInput, Cartridge* pCartridge)
{
    m_pProcessor = pProcessor;
    m_pMemory = pMemory;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_bEnabled = false;
    m_MaxAddress = 0;
    m_MinAddress = 0;
}

void MemoryRule::SetMaxAddress(u16 maxAddress)
{
    this->m_MaxAddress = maxAddress;
}

u16 MemoryRule::GetMaxAddress() const
{
    return m_MaxAddress;
}

void MemoryRule::SetMinAddress(u16 minAddress)
{
    this->m_MinAddress = minAddress;
}

u16 MemoryRule::GetMinAddress() const
{
    return m_MinAddress;
}

bool MemoryRule::IsEnabled() const
{
    return m_bEnabled;
}

void MemoryRule::Enable()
{
    m_bEnabled = true;
}

void MemoryRule::Disable()
{
    m_bEnabled = false;
}

bool MemoryRule::IsAddressInRange(u16 address)
{
    return (address >= m_MinAddress) && (address <= m_MaxAddress);
}