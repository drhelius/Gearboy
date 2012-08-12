#ifndef MEMORYRULE_H
#define	MEMORYRULE_H

#include "definitions.h"

class Memory;
class Video;
class Processor;
class Input;

class MemoryRule
{
public:
    MemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput);
    virtual u8 PerformRead(u16 address) = 0;
    virtual void PerformWrite(u16 address, u8 value) = 0;
    void SetMaxAddress(u16 maxAddress);
    u16 GetMaxAddress() const;
    void SetMinAddress(u16 minAddress);
    u16 GetMinAddress() const;
    bool IsEnabled() const;
    void Enable();
    void Disable();
    bool IsAddressInRange(u16 address);
protected:
    Processor* m_pProcessor;
    Memory* m_pMemory;
    Video* m_pVideo;
    Input* m_pInput;
private:
    bool m_bEnabled;
    u16 m_MinAddress;
    u16 m_MaxAddress;
};

#endif	/* MEMORYRULE_H */

