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

