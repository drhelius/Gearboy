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

#ifndef PROFILER_H
#define PROFILER_H

#include "definitions.h"

#define PROFILER_MAX_FUNCTIONS 8192
#define PROFILER_HASH_BITS 14
#define PROFILER_HASH_SIZE (1 << PROFILER_HASH_BITS)
#define PROFILER_MAX_DEPTH 256
#define PROFILER_ROOT 0
#define PROFILER_HALT 1
#define PROFILER_INVALID 0xFFFF
#define PROFILER_RAM_KEY 0x01000000

static_assert(PROFILER_MAX_FUNCTIONS < PROFILER_HASH_SIZE, "Profiler hash table too small");

enum GB_Profiler_Function_Type : u8
{
    PROFILER_FUNCTION_ROOT = 0,
    PROFILER_FUNCTION_HALT,
    PROFILER_FUNCTION_CALL,
    PROFILER_FUNCTION_IRQ,
};

struct GB_Profiler_Function
{
    u64 inclusive_cycles;
    u64 exclusive_cycles;
    u32 key;
    u32 calls;
    u32 active;
    u32 min_cycles;
    u32 max_cycles;
    u16 address;
    u16 bank;
    GB_Profiler_Function_Type type;
};

struct GB_Profiler_Frame
{
    u64 enter_cycle;
    u64 irq_cycles;
    u16 function;
    u16 return_sp;
    bool irq;
};

class Profiler
{
public:
    Profiler(const u64* master_clock_cycles);
    ~Profiler();
    void Reset();
    void ResetStack();
    void Enable(bool enable);
    INLINE bool IsEnabled() const;
    void Sync();
    void Enter(u32 key, u16 address, u16 bank, u16 return_sp, bool irq, u32 pending_cycles);
    void Return(u16 sp, u32 pending_cycles);
    void Halt(bool halted, u32 pending_cycles);
    const GB_Profiler_Function* GetFunctions() const;
    u32 GetFunctionCount() const;
    u64 GetTotalCycles() const;

private:
    void InitFunction(u16 index, u32 key, u16 address, u16 bank, GB_Profiler_Function_Type type);
    u16 FindFunction(u32 key, u16 address, u16 bank, GB_Profiler_Function_Type type);
    void Charge(u64 cycle);
    void Leave(u64 cycle);
    void AddSample(GB_Profiler_Function* function, u64 cycles);
    u16 GetCurrentFunction() const;

private:
    GB_Profiler_Function* m_functions;
    u16* m_hash;
    u32 m_function_count;
    GB_Profiler_Frame m_stack[PROFILER_MAX_DEPTH];
    int m_depth;
    bool m_enabled;
    bool m_halted;
    u64 m_halt_cycle;
    u64 m_last_cycle;
    u64 m_total_cycles;
    u64 m_irq_cycles;
    const u64* m_master_clock_cycles;
};

INLINE bool Profiler::IsEnabled() const
{
    return m_enabled;
}

#endif /* PROFILER_H */
