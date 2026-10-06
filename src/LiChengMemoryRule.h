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

#ifndef LICHENGMEMORYRULE_H
#define LICHENGMEMORYRULE_H

#include "MBC5LogoMemoryRule.h"

class LiChengMemoryRule : public MBC5LogoMemoryRule
{
public:
    LiChengMemoryRule(Memory* pMemory, Cartridge* pCartridge);
    virtual ~LiChengMemoryRule();
    virtual void PerformWrite(u16 address, u8 value);
    virtual u8 GetMapperType();
    virtual size_t GetRamSize();
};

#endif /* LICHENGMEMORYRULE_H */
