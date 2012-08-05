#ifndef PROCESSOR_H
#define	PROCESSOR_H

#include "definitions.h"
#include "SixteenBitRegister.h"

class Memory;

class Processor
{
public:
    Processor(Memory* pMemory);
    ~Processor();
    
private:
    void ClearAllFlags();
    void ToggleZeroFlagFromResult(u8 result);
    void SetFlag(u8 flag);
    void ToggleFlag(u8 flag);
    void UntoggleFlag(u8 flag);
    bool IsSetFlag(u8 flag);
    void OPCodes_LD(EightBitRegister* reg1, EightBitRegister reg2);
    void OPCodes_LD(EightBitRegister* reg, u16 address);
    void OPCodes_LD(u16 address, EightBitRegister reg);
    void OPCodes_OR(u8 number);
    void OPCodes_XOR(u8 number);
    void OPCodes_AND(u8 number);
    void OPCodes_CP(u8 number);
    void OPCodes_INC(EightBitRegister* reg);
    void OPCodes_INC_HL();
    void OPCodes_DEC(EightBitRegister* reg);
    void OPCodes_DEC_HL();
    void OPCodes_ADD(u8 number);
    void OPCodes_ADC(u8 number);
    void OPCodes_SUB(u8 number);
    void OPCodes_SBC(u8 number);
    void OPCodes_ADD_HL(u16 number);
    void OPCodes_ADD_SP(s8 number);
    void OPCodes_SWAP_Register(EightBitRegister* reg);
    void OPCodes_SWAP_HL();
    void OPCodes_SLA(EightBitRegister* reg);
    void OPCodes_SLA_HL();
    void OPCodes_SRA(EightBitRegister* reg);
    void OPCodes_SRA_HL();
    void OPCodes_SRL(EightBitRegister* reg);
    void OPCodes_SRL_HL();
    void OPCodes_RLC(EightBitRegister* reg);
    void OPCodes_RLC_HL();
    void OPCodes_RL(EightBitRegister* reg);
    void OPCodes_RL_HL();
    void OPCodes_RRC(EightBitRegister* reg);
    void OPCodes_RRC_HL();
    void OPCodes_RR(EightBitRegister* reg);
    void OPCodes_RR_HL();
    void OPCodes_BIT(EightBitRegister* reg, int bit);
    void OPCodes_BIT_HL(int bit);
    void OPCodes_SET(EightBitRegister* reg, int bit);
    void OPCodes_SET_HL(int bit);
    void OPCodes_RES(EightBitRegister* reg, int bit);
    void OPCodes_RES_HL(int bit);
    
#include "opcodes.h"
#include "opcodes_cb.h"

private:
    Memory* m_pMemory;
    SixteenBitRegister AF;
    SixteenBitRegister BC;
    SixteenBitRegister DE;
    SixteenBitRegister HL;
    SixteenBitRegister SP;
    SixteenBitRegister PC;

};

#endif	/* PROCESSOR_H */

