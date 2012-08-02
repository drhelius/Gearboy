#ifndef EIGHTBITREGISTER_H
#define	EIGHTBITREGISTER_H

#include "definitions.h"

class EightBitRegister
{
public:
    EightBitRegister();
    void SetValue(u8 value);
    u8 GetValue() const;
    void Increment();
    void Decrement();
private:
    u8 m_Value;
    
};

#endif	/* EIGHTBITREGISTER_H */

