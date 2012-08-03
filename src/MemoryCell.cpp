#include "MemoryCell.h"

MemoryCell::MemoryCell()
{
    Reset();
}

void MemoryCell::Reset()
{
    m_iWriteCount = 0;
    m_iReadCount = 0;
    m_szDisassembled[0] = 0;
    m_Register.SetValue(0);
    m_OldRegister.SetValue(0);
}

u8 MemoryCell::Read()
{
    m_iReadCount++;
    return m_Register.GetValue();
}

void MemoryCell::Write(u8 value)
{
    m_iWriteCount++;
    m_OldRegister.SetValue(m_Register.GetValue());
    m_Register.SetValue(value);
}

void MemoryCell::SetDisassembledString(const char* szDisassembled)
{
    strcpy(m_szDisassembled, szDisassembled);
}

char* MemoryCell::GetDisassembledString() 
{
    return m_szDisassembled;
}

int MemoryCell::GetReadCount() const
{
    return m_iReadCount;
}

int MemoryCell::GetWriteCount() const
{
    return m_iWriteCount;
}

u8 MemoryCell::GetOldValue() const
{
    return m_OldRegister.GetValue();
}

u8 MemoryCell::GetValue() const
{
    return m_Register.GetValue();
}

