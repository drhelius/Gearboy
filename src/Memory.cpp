#include "Memory.h"
#include "MemoryRule.h"
#include "MemoryCell.h"

Memory::Memory()
{
    InitPointer(m_pMap);
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
}

void Memory::Init()
{
    m_pMap = new MemoryCell[65536];
    
    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 65536; i++)
    {
        m_pMap[i].Reset();
        if (i >= 0xFF00)
            m_pMap[i].SetValue(kInitialValuesForFFXX[i - 0xFF00]);
    }
}

void Memory::AddRule(MemoryRule* pRule)
{
    m_Rules.push_back(pRule);
}

u8 Memory::Read(u16 address)
{
    return Retrieve(address);
}

void Memory::Write(u16 address, u8 value)
{
    Load(address, value);
}

u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address].Read();
}

void Memory::Load(u16 address, u8 value)
{
    m_pMap[address].Write(value);
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    m_pMap[address].SetDisassembledString(szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pMap[address].GetDisassembledString()[0] != 0;
}

