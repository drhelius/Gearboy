#ifndef SIXTEENBITREGISTER_H
#define	SIXTEENBITREGISTER_H

#include "definitions.h"
#include "EightBitRegister.h"

class SixteenBitRegister
{
public:
    SixteenBitRegister();
    void SetLow(u8 low);
    u8 GetLow() const;
    void SetHigh(u8 high);
    u8 GetHigh() const;
    void SetValue(u16 value);
    u16 GetValue() const;
    void Increment();
    void Decrement();
private:
    EightBitRegister m_High;
    EightBitRegister m_Low;

};

#endif	/* SIXTEENBITREGISTER_H */

