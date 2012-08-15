#ifndef MBC3MEMORYRULE_H
#define	MBC3MEMORYRULE_H

#include "MemoryRule.h"

class MBC3MemoryRule : public MemoryRule
{
public:
    MBC3MemoryRule(Processor* pProcessor, Memory* pMemory, 
            Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual ~MBC3MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    u8 m_HigherRomBankBits;
    u8* m_pRAMBanks;
};

#endif	/* MBC3MEMORYRULE_H */

