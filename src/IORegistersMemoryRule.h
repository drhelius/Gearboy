#ifndef IOREGISTERSMEMORYRULE_H
#define	IOREGISTERSMEMORYRULE_H

#include "MemoryRule.h"

class IORegistersMemoryRule : public MemoryRule
{
public:
    IORegistersMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo);
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);

};

#endif	/* IOREGISTERSMEMORYRULE_H */

