/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef PROCESSOR_INLINE_H
#define	PROCESSOR_INLINE_H

#include <cassert>
#include "definitions.h"
#include "log.h"
#include "Memory.h"
#include "opcode_timing.h"

INLINE void Processor::TraceInstruction(u16 pc, bool halt_bug)
{
    if (m_pTraceLogger->IsEnabled(TRACE_CPU))
        LogTraceInstruction(pc, halt_bug);
}

INLINE void Processor::TraceIRQEvent(u16 pc, u16 vector, u8 irq_type)
{
    if (m_pTraceLogger->IsEnabled(TRACE_CPU_IRQ))
        LogIRQEvent(pc, vector, irq_type);
}

INLINE void Processor::TraceTimerEvent(u8 event)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_TIMER, event))
        LogTimerEvent(event);
}

INLINE void Processor::TraceSerialEvent(u8 event)
{
    if (IsValidPointer(m_pTraceLogger) && m_pTraceLogger->IsEventEnabled(TRACE_SERIAL, event))
        LogSerialEvent(event);
}

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
}

inline u16 Processor::GetDIVCounter() const
{
    return ((u16)m_pMemory->Retrieve(0xFF04) << 8) | (m_iDIVCycles & 0xFF);
}

inline void Processor::IncrementTIMA()
{
    u8 tima = m_pMemory->Retrieve(0xFF05);

    if (tima == 0xFF)
    {
        m_pMemory->Load(0xFF05, m_pMemory->Retrieve(0xFF06));
        TraceTimerEvent(TRACE_TIMER_RELOAD);
        RequestInterrupt(Timer_Interrupt);
        TraceTimerEvent(TRACE_TIMER_IRQ_REQUEST);
    }
    else
    {
        m_pMemory->Load(0xFF05, tima + 1);
    }
}

inline void Processor::ResetDIVCycles()
{
    m_iDIVCycles = 0;
    m_iSerialDividerOffset = 0;
    m_pMemory->Load(0xFF04, 0x00);
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

NO_INLINE COLD inline void Processor::InvalidOPCode()
{
    Debug("--> ** INVALID OP Code");
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
    if ((SP.GetValue() & 0xFF) + (number & 0xFF) > 0xFF)
    {
        ToggleFlag(FLAG_CARRY);
    }
    if ((SP.GetValue() & 0xF) + (number & 0xF) > 0xF)
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
inline std::vector<Processor::GB_Breakpoint>* Processor::GetBreakpoints()
{
    return &m_breakpoints;
}

INLINE bool Processor::MemoryBreakpointsEnabled() const
{
    return m_breakpoints_enabled;
}

inline std::stack<Processor::GB_CallStackEntry>* Processor::GetDisassemblerCallStack()
{
    return &m_disassembler_call_stack;
}

inline void Processor::PushCallStack(u16 src, u16 dest, u16 back, u8 bank)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_CallStackEntry entry;
    entry.src = src;
    entry.dest = dest;
    entry.back = back;
    entry.bank = bank;
    if (m_disassembler_call_stack.size() < 256)
        m_disassembler_call_stack.push(entry);
#else
    UNUSED(src);
    UNUSED(dest);
    UNUSED(back);
    UNUSED(bank);
#endif
}

inline void Processor::PopCallStack()
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (!m_disassembler_call_stack.empty())
        m_disassembler_call_stack.pop();
#endif
}

INLINE u8 Processor::RunFor(u8 ticks)
{
    u8 executed = 0;

    assert(m_iMachineCycle == (unsigned int)(4 >> m_iSpeedMultiplier));

    while (executed < ticks)
    {
        m_iCurrentClockCycles = 0;
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
        m_cpu_breakpoint_hit = false;
        m_memory_breakpoint_hit = false;
        m_run_to_breakpoint_hit = false;
#endif

        if (m_iAccurateOPCodeState == 0 && m_bHalt)
        {
            m_iCurrentClockCycles += m_iMachineCycle;

            if (m_iUnhaltCycles > 0)
            {
                m_iUnhaltCycles -= m_iCurrentClockCycles;

                if (m_iUnhaltCycles <= 0)
                {
                    m_iUnhaltCycles = 0;
                    m_bHalt = false;
                }
            }

            if (m_bHalt && (InterruptPending() != None_Interrupt) && (m_iUnhaltCycles == 0))
            {
                m_iUnhaltCycles = AdjustedCycles(12);
            }
        }

        bool interrupt_served = false;
        bool halt_bug_active = false;

        if (!m_bHalt)
        {
            Interrupts interrupt = InterruptPending();

            if (m_bIME && (interrupt != None_Interrupt) && (m_iAccurateOPCodeState == 0))
            {
                ServeInterrupt(interrupt);
                interrupt_served = true;
            }
            else
            {
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
                if (m_iAccurateOPCodeState == 0)
                    TraceInstruction(PC.GetValue(), m_bSkipPCBug);
#endif

                u8 opcode = m_pMemory->Read(PC.GetValue());
                PC.Increment();

                if (m_bSkipPCBug)
                {
                    halt_bug_active = true;
                    PC.Decrement();
                }

                const u8* accurateOPcodes;
                const u8* machineCycles;
                OPCptr* opcodeTable;
                bool isCB = (opcode == 0xCB);

                if (isCB)
                {
                    accurateOPcodes = kOPCodeCBAccurate;
                    machineCycles = kOPCodeCBMachineCycles;
                    opcodeTable = m_OPCodesCB;

                    opcode = m_pMemory->Read(PC.GetValue());
                    PC.Increment();

                    if (m_bSkipPCBug)
                    {
                        halt_bug_active = true;
                        PC.Decrement();
                    }
                }
                else
                {
                    accurateOPcodes = kOPCodeAccurate;
                    machineCycles = kOPCodeMachineCycles;
                    opcodeTable = m_OPCodes;
                }

                if ((accurateOPcodes[opcode] != 0) && (m_iAccurateOPCodeState == 0))
                {
                    int left_cycles = (accurateOPcodes[opcode] < 3 ? 2 : 3);
                    m_iCurrentClockCycles += (machineCycles[opcode] - left_cycles) * m_iMachineCycle;
                    m_iAccurateOPCodeState = 1;

                    if (!halt_bug_active)
                    {
                        PC.Decrement();
                        if (isCB)
                            PC.Decrement();
                    }
                }
                else
                {
                    opcodeTable[opcode](this);

                    if (halt_bug_active)
                        m_bSkipPCBug = false;

                    if (m_bBranchTaken)
                    {
                        m_bBranchTaken = false;
                        m_iCurrentClockCycles += kOPCodeBranchMachineCycles[opcode] * m_iMachineCycle;
                    }
                    else
                    {
                        switch (m_iAccurateOPCodeState)
                        {
                        case 0:
                            m_iCurrentClockCycles += machineCycles[opcode] * m_iMachineCycle;
                            break;
                        case 1:
                            if (accurateOPcodes[opcode] == 3)
                            {
                                m_iCurrentClockCycles += m_iMachineCycle;
                                m_iAccurateOPCodeState = 2;
                                PC.Decrement();
                                if (isCB)
                                    PC.Decrement();
                            }
                            else
                            {
                                m_iCurrentClockCycles += 2 * m_iMachineCycle;
                                m_iAccurateOPCodeState = 0;
                            }
                            break;
                        case 2:
                            m_iCurrentClockCycles += 2 * m_iMachineCycle;
                            m_iAccurateOPCodeState = 0;
                            break;
                        }
                    }
                }
            }

            #ifndef GEARBOY_DISABLE_DISASSEMBLER
            DisassembleNextOPCode();
            #endif
        }

        if (!interrupt_served && (m_iInterruptDelayCycles > 0))
        {
            m_iInterruptDelayCycles -= m_iCurrentClockCycles;
        }

        if (!interrupt_served && (m_iAccurateOPCodeState == 0) && (m_iIMECycles > 0))
        {
            m_iIMECycles -= m_iCurrentClockCycles;

            if (m_iIMECycles <= 0)
            {
                m_iIMECycles = 0;
                m_bIME = true;
            }
        }

        executed += m_iCurrentClockCycles;
    }

    return executed;
}

INLINE void Processor::UpdateTimers(u8 ticks)
{
    m_iDIVCycles += ticks;

    unsigned int div_cycles = AdjustedCycles(256);

    while (m_iDIVCycles >= div_cycles)
    {
        m_iDIVCycles -= div_cycles;
        u8 div = m_pMemory->Retrieve(0xFF04);
        div++;
        m_pMemory->Load(0xFF04, div);
    }

    u8 tac = m_pMemory->Retrieve(0xFF07);

    // if tima is running
    if (tac & 0x04)
    {
        m_iTIMACycles += ticks;

        unsigned int freq = 0;

        switch (tac & 0x03)
        {
            case 0:
                freq = AdjustedCycles(1024);
                break;
            case 1:
                freq = AdjustedCycles(16);
                break;
            case 2:
                freq = AdjustedCycles(64);
                break;
            case 3:
                freq = AdjustedCycles(256);
                break;
        }

        while (m_iTIMACycles >= freq)
        {
            m_iTIMACycles -= freq;
            u8 tima = m_pMemory->Retrieve(0xFF05);

            if (tima == 0xFF)
            {
                tima = m_pMemory->Retrieve(0xFF06);
                m_pMemory->Load(0xFF05, tima);
                TraceTimerEvent(TRACE_TIMER_RELOAD);
                RequestInterrupt(Timer_Interrupt);
                TraceTimerEvent(TRACE_TIMER_IRQ_REQUEST);
            }
            else
            {
                tima++;
                m_pMemory->Load(0xFF05, tima);
            }
        }
    }
}

INLINE void Processor::UpdateSerial(u8 ticks, u64 current_cycle)
{
    u8 sc = m_pMemory->Retrieve(0xFF02);

    if (unlikely(m_bSerialDataWritePending || m_bSerialControlWritePending ||
        m_bSerialRestorePending || m_bSerialTransferActive ||
        m_bSerialWaitingExternal || ((sc & 0x80) != 0 && m_iSerialBit >= 0)))
    {
        UpdateSerialActive(ticks, current_cycle);
    }
}

INLINE void Processor::SynchronizeLinkCable(u64 current_cycle)
{
    if (likely(!m_bLinkCableConnected || !m_link_cable_sync_callback))
        return;

    u32 sync_cycles = GetLinkCableSyncCycles(current_cycle);

    if (current_cycle >= m_iLinkCableNextSyncCycle || sync_cycles != m_iLinkCableSyncCycles)
    {
        m_link_cable_sync_callback(current_cycle, GetLinkCablePromiseCycles(current_cycle), m_link_cable_user_data);
        m_iLinkCableSyncCycles = sync_cycles;
        m_iLinkCableNextSyncCycle = current_cycle + MAX((u32)1, sync_cycles);
    }
}

#endif	/* PROCESSOR_INLINE_H */
