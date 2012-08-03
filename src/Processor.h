#ifndef PROCESSOR_H
#define	PROCESSOR_H

#include "definitions.h"
#include "SixteenBitRegister.h"

class Processor
{
public:
    Processor();
    ~Processor();
private:
    SixteenBitRegister AF;
    SixteenBitRegister BC;
    SixteenBitRegister DE;
    SixteenBitRegister HL;
    SixteenBitRegister SP;
    SixteenBitRegister PC;

};

#endif	/* PROCESSOR_H */

