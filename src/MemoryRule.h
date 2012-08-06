#ifndef MEMORYRULE_H
#define	MEMORYRULE_H

#include "definitions.h"

class MemoryRule
{
public:

    MemoryRule();
    virtual void Perform() = 0;
    void SetMaxAddress(u16 maxAddress);
    u16 GetMaxAddress() const;
    void SetMinAddress(u16 minAddress);
    u16 GetMinAddress() const;
    bool IsEnabled() const;
    void Enable();
    void Disable();
private:
    bool m_bEnabled;
    u16 m_MinAddress;
    u16 m_MaxAddress;
};

#endif	/* MEMORYRULE_H */

