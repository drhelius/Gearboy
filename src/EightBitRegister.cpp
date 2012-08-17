/* 
 * Gearboy Gameboy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * The full license is available at http://www.gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

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

