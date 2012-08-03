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

void Memory::AddRule(MemoryRule* pRule)
{
    m_Rules.push_back(pRule);
}

