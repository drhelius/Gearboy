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

#ifndef SIXTEENBITREGISTER_H
#define	SIXTEENBITREGISTER_H

#include "definitions.h"
#include "EightBitRegister.h"

class SixteenBitRegister
{
public:
    SixteenBitRegister() { }
    void SetLow(u8 low);
    u8 GetLow() const;
    void SetHigh(u8 high);
    u8 GetHigh() const;
    EightBitRegister* GetHighRegister();
    EightBitRegister* GetLowRegister();
    void SetValue(u16 value);
    u16 GetValue() const;
    void Increment();
    void Decrement();

private:
    EightBitRegister m_High;
    EightBitRegister m_Low;
};


inline void SixteenBitRegister::SetLow(u8 low)
{
    this->m_Low.SetValue(low);
}

inline u8 SixteenBitRegister::GetLow() const
{
    return m_Low.GetValue();
}

inline void SixteenBitRegister::SetHigh(u8 high)
{
    this->m_High.SetValue(high);
}

inline u8 SixteenBitRegister::GetHigh() const
{
    return m_High.GetValue();
}

inline EightBitRegister* SixteenBitRegister::GetHighRegister()
{
    return &m_High;
}

inline EightBitRegister* SixteenBitRegister::GetLowRegister()
{
    return &m_Low;
}

inline void SixteenBitRegister::SetValue(u16 value)
{
    m_Low.SetValue((u8) (value & 0xFF));
    m_High.SetValue((u8) ((value >> 8) & 0xFF));
}

inline u16 SixteenBitRegister::GetValue() const
{
    u8 high = m_High.GetValue();
    u8 low = m_Low.GetValue();

    return (high << 8) + low;
}

inline void SixteenBitRegister::Increment()
{
    u16 value = this->GetValue();
    value++;
    this->SetValue(value);
}

inline void SixteenBitRegister::Decrement()
{
    u16 value = this->GetValue();
    value--;
    this->SetValue(value);
}

#endif	/* SIXTEENBITREGISTER_H */
