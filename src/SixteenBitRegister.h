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
    SixteenBitRegister();
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

#endif	/* SIXTEENBITREGISTER_H */

