#ifndef MEMORYRULE_H
#define	MEMORYRULE_H

#include "definitions.h"

class MemoryRule
{
public:

    MemoryRule()
    {
        m_bEnabled=false;
        m_MaxAddress=0;
        m_MinAddress=0;   
    }

    virtual void Perform() = 0;

    void SetMaxAddress(u16 maxAddress)
    {
        this->m_MaxAddress = maxAddress;
    }

    u16 GetMaxAddress() const
    {
        return m_MaxAddress;
    }

    void SetMinAddress(u16 minAddress)
    {
        this->m_MinAddress = minAddress;
    }

    u16 GetMinAddress() const
    {
        return m_MinAddress;
    }

    bool IsEnabled() const
    {
        return m_bEnabled;
    }

    void Enable()
    {
        m_bEnabled = true;
    }

    void Disable()
    {
        m_bEnabled = false;
    }
private:
    bool m_bEnabled;
    u16 m_MinAddress;
    u16 m_MaxAddress;

};

#endif	/* MEMORYRULE_H */

