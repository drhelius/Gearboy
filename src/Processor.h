#ifndef PROCESSOR_H
#define	PROCESSOR_H

#include "definitions.h"
#include "SixteenBitRegister.h"

class Processor
{
public:
    Processor();
    ~Processor();
    void ClearAllFlags();
    void ToggleZeroFlagFromResult(u8 result);
    void SetFlag(u8 flag);
    void ToggleFlag(u8 flag);
    void UntoggleFlag(u8 flag);
    bool IsSetFlag(u8 flag);
     
private:
    SixteenBitRegister AF;
    SixteenBitRegister BC;
    SixteenBitRegister DE;
    SixteenBitRegister HL;
    SixteenBitRegister SP;
    SixteenBitRegister PC;

};

#endif	/* PROCESSOR_H */

