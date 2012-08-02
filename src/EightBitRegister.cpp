#include "EightBitRegister.h"

EightBitRegister::EightBitRegister()
{
    m_Value = 0;
}

void EightBitRegister::SetValue(u8 value)
{
    this->m_Value = value;
}

u8 EightBitRegister::GetValue() const
{
    return m_Value;
}

void EightBitRegister::Increment()
{
    m_Value++;
}
    
void EightBitRegister::Decrement()
{
    m_Value--;
}

