#ifndef PROCESSOR_INLINE_H
#define	PROCESSOR_INLINE_H

#include "definitions.h"
#include "Memory.h"

inline void Processor::ClearAllFlags()
{
    SetFlag(FLAG_NONE);
}

inline void Processor::ToggleZeroFlagFromResult(u8 result)
{
    if (result == 0)
        ToggleFlag(FLAG_ZERO);
}

inline void Processor::SetFlag(u8 flag)
{
    AF.SetLow(flag);
}

inline void Processor::FlipFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() ^ flag);
}

inline void Processor::ToggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() | flag);
}

inline void Processor::UntoggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() & (~flag));
}

inline bool Processor::IsSetFlag(u8 flag)
{
    return (AF.GetLow() & flag) != 0;
}

inline void Processor::StackPush(SixteenBitRegister* reg)
{
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetHigh());
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetLow());
}

inline void Processor::StackPop(SixteenBitRegister* reg)
{
    reg->SetLow(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
    reg->SetHigh(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
}

inline void Processor::InvalidOPCode()
{
    Log("--> ** INVALID OP Code");
}

inline void Processor::OPCodes_LD(EightBitRegister* reg1, u8 reg2)
{
    reg1->SetValue(reg2);
}

inline void Processor::OPCodes_LD(EightBitRegister* reg, u16 address)
{
    reg->SetValue(m_pMemory->Read(address));
}

inline void Processor::OPCodes_LD(u16 address, u8 reg)
{
    m_pMemory->Write(address, reg);
}

inline void Processor::OPCodes_OR(u8 number)
{
    u8 result = AF.GetHigh() | number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_XOR(u8 number)
{
    u8 result = AF.GetHigh() ^ number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_AND(u8 number)
{
    u8 result = AF.GetHigh() & number;
    AF.SetHigh(result);
    SetFlag(FLAG_HALF);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_CP(u8 number)
{
    SetFlag(FLAG_SUB);
    if (AF.GetHigh() < number)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if (AF.GetHigh() == number)
    {
        ToggleFlag(FLAG_ZERO);
    }
    if (((AF.GetHigh() - number) & 0xF) > (AF.GetHigh() & 0xF))
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_INC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() + 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_INC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue()) + 1;
    m_pMemory->Write(HL.GetValue(), result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_DEC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() - 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x0F)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_DEC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue()) - 1;
    m_pMemory->Write(HL.GetValue(), result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x0F)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_ADD(u8 number)
{
    int result = AF.GetHigh() + number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    AF.SetHigh(static_cast<u8> (result));
    ClearAllFlags();
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_ADC(u8 number)
{
    int carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    int result = AF.GetHigh() + number + carry;
    ClearAllFlags();
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if (result > 0xFF)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if (((AF.GetHigh()& 0x0F) + (number & 0x0F) + carry) > 0x0F)
    {
        ToggleFlag(FLAG_HALF);
    }
    AF.SetHigh(static_cast<u8> (result));
}

inline void Processor::OPCodes_SUB(u8 number)
{
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    AF.SetHigh(static_cast<u8> (result));
    SetFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if ((carrybits & 0x100) != 0)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((carrybits & 0x10) != 0)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_SBC(u8 number)
{
    int carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    int result = AF.GetHigh() - number - carry;
    SetFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(static_cast<u8> (result));
    if (result < 0)
        ToggleFlag(FLAG_CARRY);

    if (((AF.GetHigh() & 0x0F) - (number & 0x0F) - carry) < 0)
        ToggleFlag(FLAG_HALF);
    AF.SetHigh(static_cast<u8> (result));
}

inline void Processor::OPCodes_ADD_HL(u16 number)
{
    int result = HL.GetValue() + number;
    IsSetFlag(FLAG_ZERO) ? SetFlag(FLAG_ZERO) : ClearAllFlags();
    if (result & 0x10000)
        ToggleFlag(FLAG_CARRY);

    if ((HL.GetValue() ^ number ^ (result & 0xFFFF)) & 0x1000)
        ToggleFlag(FLAG_HALF);
    HL.SetValue(static_cast<u16> (result));
}

inline void Processor::OPCodes_ADD_SP(s8 number)
{
    int result = SP.GetValue() + number;
    ClearAllFlags();
    if (((SP.GetValue() ^ number ^ (result & 0xFFFF)) & 0x100) == 0x100)
        ToggleFlag(FLAG_CARRY);

    if (((SP.GetValue() ^ number ^ (result & 0xFFFF)) & 0x10) == 0x10)
        ToggleFlag(FLAG_HALF);
    SP.SetValue(static_cast<u16> (result));
}

inline void Processor::OPCodes_SWAP_Register(EightBitRegister* reg)
{
    u8 low_half = reg->GetValue() & 0x0F;
    u8 high_half = (reg->GetValue() >> 4) & 0x0F;
    reg->SetValue((low_half << 4) + high_half);
    ClearAllFlags();
    ToggleZeroFlagFromResult(reg->GetValue());
}

inline void Processor::OPCodes_SWAP_HL()
{
    u8 number = m_pMemory->Read(HL.GetValue());
    u8 low_half = number & 0x0F;
    u8 high_half = (number >> 4) & 0x0F;
    number = (low_half << 4) + high_half;
    m_pMemory->Write(HL.GetValue(), number);
    ClearAllFlags();
    ToggleZeroFlagFromResult(number);
}

inline void Processor::OPCodes_SLA(EightBitRegister* reg)
{
    (reg->GetValue() & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 result = reg->GetValue() << 1;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SLA_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRA(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        result >>= 1;
    }
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRA_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        result >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRL(EightBitRegister* reg)
{
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    reg->SetValue(result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRL_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RLC(EightBitRegister* reg, bool isRegisterA)
{
    u8 result = reg->GetValue();
    if ((result & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        ClearAllFlags();
        result <<= 1;
    }
    reg->SetValue(result);

    if (!isRegisterA)
        ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RLC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    if ((result & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        ClearAllFlags();
        result <<= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RL(EightBitRegister* reg, bool isRegisterA)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = reg->GetValue();
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    reg->SetValue(result);

    if (!isRegisterA)
        ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RL_HL()
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = m_pMemory->Read(HL.GetValue());
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RRC(EightBitRegister* reg, bool isRegisterA)
{
    u8 result = reg->GetValue();
    if ((result & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        ClearAllFlags();
        result >>= 1;
    }
    reg->SetValue(result);

    if (!isRegisterA)
        ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RRC_HL()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    if ((result & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        ClearAllFlags();
        result >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RR(EightBitRegister* reg, bool isRegisterA)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = reg->GetValue();
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    reg->SetValue(result);

    if (!isRegisterA)
        ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_RR_HL()
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = m_pMemory->Read(HL.GetValue());
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    m_pMemory->Write(HL.GetValue(), result);
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_BIT(EightBitRegister* reg, int bit)
{
    if (((reg->GetValue() >> bit) & 0x01) == 0)
        ToggleFlag(FLAG_ZERO);

    else
        UntoggleFlag(FLAG_ZERO);
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

inline void Processor::OPCodes_BIT_HL(int bit)
{
    if (((m_pMemory->Read(HL.GetValue()) >> bit) & 0x01) == 0)
        ToggleFlag(FLAG_ZERO);

    else
        UntoggleFlag(FLAG_ZERO);
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

inline void Processor::OPCodes_SET(EightBitRegister* reg, int bit)
{
    reg->SetValue(reg->GetValue() | (0x1 << bit));
}

inline void Processor::OPCodes_SET_HL(int bit)
{
    u8 result = m_pMemory->Read(HL.GetValue());
    result |= (0x1 << bit);
    m_pMemory->Write(HL.GetValue(), result);
}

inline void Processor::OPCodes_RES(EightBitRegister* reg, int bit)
{
    reg->SetValue(reg->GetValue() & (~(0x1 << bit)));
}

inline void Processor::OPCodes_RES_HL(int bit)
{
    u8 result = m_pMemory->Read(HL.GetValue());
    result &= ~(0x1 << bit);
    m_pMemory->Write(HL.GetValue(), result);
}

#endif	/* PROCESSOR_INLINE_H */

