#ifndef ROMONLYMEMORYRULE_H
#define	ROMONLYMEMORYRULE_H

#include "MemoryRule.h"

class RomOnlyMemoryRule : public MemoryRule
{
public:
    RomOnlyMemoryRule(Processor* pProcessor, Memory* pMemory, 
            Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
};

#endif	/* ROMONLYMEMORYRULE_H */

