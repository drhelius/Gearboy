#ifndef MBC2MEMORYRULE_H
#define	MBC2MEMORYRULE_H

#include "MemoryRule.h"

class MBC2MemoryRule : public MemoryRule
{
public:
    MBC2MemoryRule(Processor* pProcessor, Memory* pMemory, 
            Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
private:
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    u8 m_HigherRomBankBits;
};

#endif	/* MBC2MEMORYRULE_H */

