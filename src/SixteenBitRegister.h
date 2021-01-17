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

class SixteenBitRegister
{
public:
    SixteenBitRegister() { }
    void SetLow(u8 low);
    u8 GetLow() const;
    void SetHigh(u8 high);
    u8 GetHigh() const;
    u8* GetHighRegister();
    u8* GetLowRegister();
    void SetValue(u16 value);
    u16 GetValue() const;
    void Increment();
    void Decrement();

private:
    union sixteenBit
    {
        u16 v;
        struct 
        {
#ifdef IS_LITTLE_ENDIAN
            uint8_t low;
            uint8_t high;
#else
            uint8_t high;
            uint8_t low;
#endif
        };
    } m_Value;
};


inline void SixteenBitRegister::SetLow(u8 low)
{
    m_Value.low = low;
}

inline u8 SixteenBitRegister::GetLow() const
{
    return m_Value.low;
}

inline void SixteenBitRegister::SetHigh(u8 high)
{
    m_Value.high = high;
}

inline u8 SixteenBitRegister::GetHigh() const
{
    return m_Value.high;
}

inline u8* SixteenBitRegister::GetHighRegister()
{
    return &m_Value.high;
}

inline u8* SixteenBitRegister::GetLowRegister()
{
    return &m_Value.low;
}

inline void SixteenBitRegister::SetValue(u16 value)
{
    m_Value.v = value;
}

inline u16 SixteenBitRegister::GetValue() const
{
    return m_Value.v;
}

inline void SixteenBitRegister::Increment()
{
    m_Value.v++;
}

inline void SixteenBitRegister::Decrement()
{
    m_Value.v--;
}

#endif	/* SIXTEENBITREGISTER_H */

