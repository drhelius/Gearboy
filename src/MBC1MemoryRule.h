#ifndef MBC1MEMORYRULE_H
#define	MBC1MEMORYRULE_H

#include "MemoryRule.h"

class MBC1MemoryRule : public MemoryRule
{
public:
    MBC1MemoryRule(Processor* pProcessor, Memory* pMemory, 
            Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual ~MBC1MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
private:
    int m_iMode;
    int m_iCurrentRAMBank;
    bool m_bRamEnabled;
    u8 m_HigherRomBankBits;
    u8* m_pRAMBanks;
};

#endif	/* MBC1MEMORYRULE_H */

