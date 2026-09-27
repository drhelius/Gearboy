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

#include "MBC5MemoryRule.h"

class LiChengMemoryRule : public MBC5MemoryRule
{
public:
    LiChengMemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~LiChengMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual bool MapsROMDirectly();
    virtual u8 GetMapperType();
    virtual bool NeedsHighMemoryAccessNotifications();
    virtual void NotifyHighMemoryWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual size_t GetRamSize();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    enum LogoMode
    {
        LogoModeDMG,
        LogoModeCGB,
        LogoModeUnlocked,
        LogoModeDone
    };

    LogoMode m_LogoMode;
    u8 m_LogoCount;
};

#endif /* LICHENGMEMORYRULE_H */
