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

#include "SixteenBitRegister.h"

SixteenBitRegister::SixteenBitRegister()
{
}

void SixteenBitRegister::SetLow(u8 low)
{
    this->m_Low.SetValue(low);
}

u8 SixteenBitRegister::GetLow() const
{
    return m_Low.GetValue();
}

void SixteenBitRegister::SetHigh(u8 high)
{
    this->m_High.SetValue(high);
}

u8 SixteenBitRegister::GetHigh() const
{
    return m_High.GetValue();
}

EightBitRegister* SixteenBitRegister::GetHighRegister()
{
    return &m_High;
}

EightBitRegister* SixteenBitRegister::GetLowRegister()
{
    return &m_Low;
}

void SixteenBitRegister::SetValue(u16 value)
{
    m_Low.SetValue((u8) (value & BIT_MASK_8));
    m_High.SetValue((u8) ((value >> 8) & BIT_MASK_8));
}

u16 SixteenBitRegister::GetValue() const
{
    u8 high = m_High.GetValue();
    u8 low = m_Low.GetValue();

    return (high << 8) + low;
}

void SixteenBitRegister::Increment()
{
    u16 value = this->GetValue();
    value++;
    this->SetValue(value);
}

void SixteenBitRegister::Decrement()
{
    u16 value = this->GetValue();
    value--;
    this->SetValue(value);
}

