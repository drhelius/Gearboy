#ifndef PROCESSOR_INLINE_H
#define	PROCESSOR_INLINE_H

#include "definitions.h"
#include "Memory.h"

inline bool Processor::InterruptIsAboutToRaise()
{
    u8 ie_reg = m_pMemory->Retrieve(0xFFFF);
    u8 if_reg = m_pMemory->Retrieve(0xFF0F);

    return (if_reg & ie_reg & 0x1F) != 0;
}

inline Processor::Interrupts Processor::InterruptPending()
{
    u8 ie_reg = m_pMemory->Retrieve(0xFFFF);
    u8 if_reg = m_pMemory->Retrieve(0xFF0F);
    u8 ie_if = if_reg & ie_reg;
    
    if ((ie_if & 0x1F) == 0)
    {
        return None_Interrupt;
    }
    else if ((ie_if & 0x01) && (m_iInterruptDelayCycles <= 0))
    {
        return VBlank_Interrupt;
    }
    else if (ie_if & 0x02)
    {
        return LCDSTAT_Interrupt;
    }
    else if (ie_if & 0x04)
    {
        return Timer_Interrupt;
    }
    else if (ie_if & 0x08)
    {
        return Serial_Interrupt;
    }
    else if (ie_if & 0x10)
    {
        return Joypad_Interrupt;
    }
    
    return None_Interrupt;
}

inline void Processor::RequestInterrupt(Interrupts interrupt)
{
    m_pMemory->Load(0xFF0F, m_pMemory->Retrieve(0xFF0F) | interrupt);

    if ((interrupt == VBlank_Interrupt) && !m_bCGBSpeed)
    {
        m_iInterruptDelayCycles = 4;
    }
}

inline void Processor::ResetTIMACycles()
{
    m_iTIMACycles = 0;
    m_pMemory->Load(0xFF05, m_pMemory->Retrieve(0xFF06));
}

inline void Processor::ResetDIVCycles()
{
    m_iDIVCycles = 0;
    m_pMemory->Load(0xFF04, 0x00);
}

inline bool Processor::Halted() const
{
    return m_bHalt;
}

inline bool Processor::DuringOpCode() const
{
    return m_iAccurateOPCodeState != 0;
}

inline bool Processor::CGBSpeed() const
{
    return m_bCGBSpeed;
}

inline void Processor::AddCycles(unsigned int cycles)
{
    m_iCurrentClockCycles += cycles;
}

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

inline int Processor::AdjustedCycles(int cycles)
{
    if (!cycles) return cycles;
    return cycles >> m_iSpeedMultiplier;
}

inline void Processor::InvalidOPCode()
{
    Log("--> ** INVALID OP Code");
}

inline void Processor::OPCodes_LD(u8* reg1, u8 reg2)
{
    *reg1 = reg2;
}

inline void Processor::OPCodes_LD(u8* reg, u16 address)
{
    *reg = m_pMemory->Read(address);
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

inline void Processor::OPCodes_INC(u8* reg)
{
    u8 result = *reg + 1;
    *reg = result;
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    if ((result & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_INC_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue()) + 1;
        return;
    }
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(m_iReadCache);
    if ((m_iReadCache & 0x0F) == 0x00)
    {
        ToggleFlag(FLAG_HALF);
    }
}

inline void Processor::OPCodes_DEC(u8* reg)
{
    u8 result = *reg - 1;
    *reg = result;
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
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue()) - 1;
        return;
    }
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_SUB);
    ToggleZeroFlagFromResult(m_iReadCache);
    if ((m_iReadCache & 0x0F) == 0x0F)
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
    {
        ToggleFlag(FLAG_CARRY);
    }
    if (((AF.GetHigh() & 0x0F) - (number & 0x0F) - carry) < 0)
    {
        ToggleFlag(FLAG_HALF);
    }
    AF.SetHigh(static_cast<u8> (result));
}

inline void Processor::OPCodes_ADD_HL(u16 number)
{
    int result = HL.GetValue() + number;
    IsSetFlag(FLAG_ZERO) ? SetFlag(FLAG_ZERO) : ClearAllFlags();
    if (result & 0x10000)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((HL.GetValue() ^ number ^ (result & 0xFFFF)) & 0x1000)
    {
        ToggleFlag(FLAG_HALF);
    }
    HL.SetValue(static_cast<u16> (result));
}

inline void Processor::OPCodes_ADD_SP(s8 number)
{
    int result = SP.GetValue() + number;
    ClearAllFlags();
    if (((SP.GetValue() ^ number ^ (result & 0xFFFF)) & 0x100) == 0x100)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if (((SP.GetValue() ^ number ^ (result & 0xFFFF)) & 0x10) == 0x10)
    {
        ToggleFlag(FLAG_HALF);
    }
    SP.SetValue(static_cast<u16> (result));
}

inline void Processor::OPCodes_SWAP_Register(u8* reg)
{
    u8 low_half = *reg & 0x0F;
    u8 high_half = (*reg >> 4) & 0x0F;
    *reg = (low_half << 4) + high_half;
    ClearAllFlags();
    ToggleZeroFlagFromResult(*reg);
}

inline void Processor::OPCodes_SWAP_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    u8 low_half = m_iReadCache & 0x0F;
    u8 high_half = (m_iReadCache >> 4) & 0x0F;
    m_iReadCache = (low_half << 4) + high_half;
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ClearAllFlags();
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_SLA(u8* reg)
{
    (*reg & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 result = *reg << 1;
    *reg = result;
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SLA_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    (m_iReadCache & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    m_iReadCache <<= 1;
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_SRA(u8* reg)
{
    u8 result = *reg;
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
    *reg = result;
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRA_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    (m_iReadCache & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((m_iReadCache & 0x80) != 0)
    {
        m_iReadCache >>= 1;
        m_iReadCache |= 0x80;
    }
    else
    {
        m_iReadCache >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_SRL(u8* reg)
{
    u8 result = *reg;
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    *reg = result;
    ToggleZeroFlagFromResult(result);
}

inline void Processor::OPCodes_SRL_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    (m_iReadCache & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    m_iReadCache >>= 1;
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_RLC(u8* reg, bool isRegisterA)
{
    u8 result = *reg;
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
    *reg = result;
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RLC_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    if ((m_iReadCache & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        m_iReadCache <<= 1;
        m_iReadCache |= 0x1;
    }
    else
    {
        ClearAllFlags();
        m_iReadCache <<= 1;
    }
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_RL(u8* reg, bool isRegisterA)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = *reg;
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    *reg = result;
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RL_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    ((m_iReadCache & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    m_iReadCache <<= 1;
    m_iReadCache |= carry;
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_RRC(u8* reg, bool isRegisterA)
{
    u8 result = *reg;
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
    *reg = result;
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RRC_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    if ((m_iReadCache & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        m_iReadCache >>= 1;
        m_iReadCache |= 0x80;
    }
    else
    {
        ClearAllFlags();
        m_iReadCache >>= 1;
    }
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_RR(u8* reg, bool isRegisterA)
{
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = *reg;
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    *reg = result;
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RR_HL()
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    ((m_iReadCache & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    m_iReadCache >>= 1;
    m_iReadCache |= carry;
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
    ToggleZeroFlagFromResult(m_iReadCache);
}

inline void Processor::OPCodes_BIT(u8* reg, int bit)
{
    if (((*reg >> bit) & 0x01) == 0)
    {
        ToggleFlag(FLAG_ZERO);
    }
    else
    {
        UntoggleFlag(FLAG_ZERO);
    }
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

inline void Processor::OPCodes_BIT_HL(int bit)
{
    if (((m_pMemory->Read(HL.GetValue()) >> bit) & 0x01) == 0)
    {
        ToggleFlag(FLAG_ZERO);
    }
    else
    {
        UntoggleFlag(FLAG_ZERO);
    }
    ToggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

inline void Processor::OPCodes_SET(u8* reg, int bit)
{
    *reg = (*reg | (0x1 << bit));
}

inline void Processor::OPCodes_SET_HL(int bit)
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    m_iReadCache |= (0x1 << bit);
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
}

inline void Processor::OPCodes_RES(u8* reg, int bit)
{
    *reg = (*reg & (~(0x1 << bit)));
}

inline void Processor::OPCodes_RES_HL(int bit)
{
    if (m_iAccurateOPCodeState == 1)
    {
        m_iReadCache = m_pMemory->Read(HL.GetValue());
        return;
    }
    m_iReadCache &= ~(0x1 << bit);
    m_pMemory->Write(HL.GetValue(), m_iReadCache);
}

#endif	/* PROCESSOR_INLINE_H */

