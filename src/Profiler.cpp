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

#include "Profiler.h"
#include <new>

Profiler::Profiler(const u64* master_clock_cycles)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    m_functions = new (std::nothrow) GB_Profiler_Function[PROFILER_MAX_FUNCTIONS];
    m_hash = new (std::nothrow) u16[PROFILER_HASH_SIZE];
#else
    m_functions = NULL;
    m_hash = NULL;
#endif
    m_master_clock_cycles = master_clock_cycles;
    m_function_count = 0;
    m_depth = 0;
    m_enabled = false;
    m_halted = false;
    m_halt_cycle = 0;
    m_last_cycle = 0;
    m_total_cycles = 0;
    m_irq_cycles = 0;
    Reset();
}

Profiler::~Profiler()
{
    SafeDeleteArray(m_hash);
    SafeDeleteArray(m_functions);
}

void Profiler::Reset()
{
    m_function_count = 0;
    m_total_cycles = 0;

    if (IsValidPointer(m_hash))
    {
        for (int i = 0; i < PROFILER_HASH_SIZE; i++)
            m_hash[i] = PROFILER_INVALID;
    }

    if (IsValidPointer(m_functions))
    {
        InitFunction(PROFILER_ROOT, 0, 0, 0, PROFILER_FUNCTION_ROOT);
        InitFunction(PROFILER_HALT, 0, 0, 0, PROFILER_FUNCTION_HALT);
    }

    m_depth = 0;
    ResetStack();
}

void Profiler::ResetStack()
{
    for (int i = 0; i < m_depth; i++)
        m_functions[m_stack[i].function].active = 0;

    if (IsValidPointer(m_functions))
        m_functions[PROFILER_HALT].active = 0;

    m_depth = 0;
    m_halted = false;
    m_halt_cycle = 0;
    m_irq_cycles = 0;
    m_last_cycle = IsValidPointer(m_master_clock_cycles) ? *m_master_clock_cycles : 0;
}

void Profiler::Enable(bool enable)
{
    if (!IsValidPointer(m_functions) || !IsValidPointer(m_hash) || !IsValidPointer(m_master_clock_cycles))
        enable = false;

    if (enable == m_enabled)
        return;

    Sync();
    m_enabled = enable;
    ResetStack();
}

void Profiler::Sync()
{
    if (m_enabled)
        Charge(*m_master_clock_cycles);
}

void Profiler::Enter(u32 key, u16 address, u16 bank, u16 return_sp, bool irq, u32 pending_cycles)
{
    u64 cycle = *m_master_clock_cycles + pending_cycles;
    Charge(cycle);

    while ((m_depth > 0) && (m_stack[m_depth - 1].return_sp <= return_sp))
        Leave(cycle);

    if (m_depth >= PROFILER_MAX_DEPTH)
        return;

    u16 index = FindFunction(key, address, bank, irq ? PROFILER_FUNCTION_IRQ : PROFILER_FUNCTION_CALL);
    if (index == PROFILER_INVALID)
        return;

    GB_Profiler_Function* function = &m_functions[index];
    function->calls++;
    function->active++;

    GB_Profiler_Frame* frame = &m_stack[m_depth];
    frame->enter_cycle = cycle;
    frame->irq_cycles = m_irq_cycles;
    frame->function = index;
    frame->return_sp = return_sp;
    frame->irq = irq;
    m_depth++;
}

void Profiler::Return(u16 sp, u32 pending_cycles)
{
    u64 cycle = *m_master_clock_cycles + pending_cycles;
    Charge(cycle);

    while ((m_depth > 0) && (m_stack[m_depth - 1].return_sp < sp))
        Leave(cycle);

    if ((m_depth > 0) && (m_stack[m_depth - 1].return_sp == sp))
        Leave(cycle);
}

void Profiler::Halt(bool halted, u32 pending_cycles)
{
    if (halted == m_halted)
        return;

    u64 cycle = *m_master_clock_cycles + pending_cycles;
    Charge(cycle);

    GB_Profiler_Function* function = &m_functions[PROFILER_HALT];

    if (halted)
    {
        function->calls++;
        function->active = 1;
        m_halt_cycle = cycle;
    }
    else
    {
        u64 cycles = (cycle > m_halt_cycle) ? cycle - m_halt_cycle : 0;
        function->active = 0;
        function->inclusive_cycles += cycles;
        AddSample(function, cycles);
    }

    m_halted = halted;
}

const GB_Profiler_Function* Profiler::GetFunctions() const
{
    return m_functions;
}

u32 Profiler::GetFunctionCount() const
{
    return m_function_count;
}

u64 Profiler::GetTotalCycles() const
{
    return m_total_cycles;
}

void Profiler::InitFunction(u16 index, u32 key, u16 address, u16 bank, GB_Profiler_Function_Type type)
{
    GB_Profiler_Function* function = &m_functions[index];
    function->inclusive_cycles = 0;
    function->exclusive_cycles = 0;
    function->key = key;
    function->calls = 0;
    function->active = 0;
    function->min_cycles = 0xFFFFFFFF;
    function->max_cycles = 0;
    function->address = address;
    function->bank = bank;
    function->type = type;

    if (index >= m_function_count)
        m_function_count = index + 1;
}

u16 Profiler::FindFunction(u32 key, u16 address, u16 bank, GB_Profiler_Function_Type type)
{
    u32 slot = (key * 2654435761U) >> (32 - PROFILER_HASH_BITS);

    while (true)
    {
        u16 index = m_hash[slot];

        if (index == PROFILER_INVALID)
        {
            if (m_function_count >= PROFILER_MAX_FUNCTIONS)
                return PROFILER_INVALID;

            index = (u16)m_function_count;
            InitFunction(index, key, address, bank, type);
            m_hash[slot] = index;
            return index;
        }

        if (m_functions[index].key == key)
            return index;

        slot = (slot + 1) & (PROFILER_HASH_SIZE - 1);
    }
}

void Profiler::Charge(u64 cycle)
{
    if (cycle <= m_last_cycle)
        return;

    u64 cycles = cycle - m_last_cycle;
    m_functions[GetCurrentFunction()].exclusive_cycles += cycles;
    m_total_cycles += cycles;
    m_last_cycle = cycle;
}

void Profiler::Leave(u64 cycle)
{
    m_depth--;
    GB_Profiler_Frame* frame = &m_stack[m_depth];
    GB_Profiler_Function* function = &m_functions[frame->function];

    u64 elapsed = (cycle > frame->enter_cycle) ? cycle - frame->enter_cycle : 0;
    u64 irq_cycles = m_irq_cycles - frame->irq_cycles;
    u64 cycles = (elapsed > irq_cycles) ? elapsed - irq_cycles : 0;

    function->active--;
    if (function->active == 0)
        function->inclusive_cycles += cycles;

    AddSample(function, cycles);

    if (frame->irq)
        m_irq_cycles += cycles;
}

void Profiler::AddSample(GB_Profiler_Function* function, u64 cycles)
{
    u32 value = (cycles > 0xFFFFFFFF) ? 0xFFFFFFFF : (u32)cycles;

    if (value < function->min_cycles)
        function->min_cycles = value;
    if (value > function->max_cycles)
        function->max_cycles = value;
}

u16 Profiler::GetCurrentFunction() const
{
    if (m_halted)
        return PROFILER_HALT;
    if (m_depth > 0)
        return m_stack[m_depth - 1].function;
    return PROFILER_ROOT;
}
