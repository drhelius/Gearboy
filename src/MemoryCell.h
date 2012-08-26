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

#ifndef MEMORYCELL_H
#define	MEMORYCELL_H

#include "definitions.h"
#include "EightBitRegister.h"

class MemoryCell
{
public:
    MemoryCell();
    void Reset();
    u8 Read();
    void Write(u8 value);
    void SetDisassembledString(const char* szDisassembled);
    char* GetDisassembledString();
    unsigned int GetReadCount() const;
    unsigned int GetWriteCount() const;
    u8 GetValue() const;
    void SetValue(u8 value);
private:
    EightBitRegister m_Register;
    char m_szDisassembled[MAX_STRING_SIZE];
};

#endif	/* MEMORYCELL_H */

