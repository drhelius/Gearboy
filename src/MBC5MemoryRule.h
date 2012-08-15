#ifndef MBC5MEMORYRULE_H
#define	MBC5MEMORYRULE_H

#include "MemoryRule.h"

class MBC5MemoryRule : public MemoryRule
{
public:
    MBC5MemoryRule(Processor* pProcessor, Memory* pMemory, 
            Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual ~MBC5MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    int m_iCurrentROMBankHi;
    bool m_bRamEnabled;
    u8 m_HigherRomBankBits;
    u8* m_pRAMBanks;
};


#endif	/* MBC5MEMORYRULE_H */

