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

#ifndef MBC5LOGOMEMORYRULE_H
#define MBC5LOGOMEMORYRULE_H

#include "MBC5MemoryRule.h"

class MBC5LogoMemoryRule : public MBC5MemoryRule
{
public:
    MBC5LogoMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MBC5LogoMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual u8 DebugRead(u16 address);
    virtual bool MapsROMDirectly();
    virtual bool NeedsHighMemoryAccessNotifications();
    virtual void NotifyHighMemoryWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
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

#endif /* MBC5LOGOMEMORYRULE_H */
