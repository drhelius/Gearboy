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

#ifndef EIGHTBITREGISTER_H
#define	EIGHTBITREGISTER_H

#include "definitions.h"

class EightBitRegister
{
public:
    EightBitRegister();
    void SetValue(u8 value);
    u8 GetValue() const;
    void Increment();
    void Decrement();
private:
    u8 m_Value;
};

#endif	/* EIGHTBITREGISTER_H */

