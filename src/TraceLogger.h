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

#ifndef TRACE_LOGGER_H
#define TRACE_LOGGER_H

#include "definitions.h"

#define TRACE_BUFFER_SIZE 100000

enum GB_Trace_Type : u8
{
    TRACE_CPU = 0,
    TRACE_CPU_IRQ,
    TRACE_LCD,
    TRACE_INPUT,
    TRACE_TIMER,
    TRACE_APU,
    TRACE_SERIAL,
    TRACE_MAPPER,
    TRACE_TYPE_COUNT,
};

static_assert(TRACE_TYPE_COUNT < 32, "Trace category count exceeds flag width");

#define TRACE_FLAG_CPU          (1U << TRACE_CPU)
#define TRACE_FLAG_CPU_IRQ      (1U << TRACE_CPU_IRQ)
#define TRACE_FLAG_LCD          (1U << TRACE_LCD)
#define TRACE_FLAG_INPUT        (1U << TRACE_INPUT)
#define TRACE_FLAG_TIMER        (1U << TRACE_TIMER)
#define TRACE_FLAG_APU          (1U << TRACE_APU)
#define TRACE_FLAG_SERIAL       (1U << TRACE_SERIAL)
#define TRACE_FLAG_MAPPER       (1U << TRACE_MAPPER)
#define TRACE_FLAG_ALL          ((1U << TRACE_TYPE_COUNT) - 1)

enum GB_Trace_LCD_Event : u8
{
    TRACE_LCD_REG_WRITE = 0,
    TRACE_LCD_VBLANK_IRQ,
    TRACE_LCD_STAT_IRQ,
    TRACE_LCD_OAM_DMA_START,
    TRACE_LCD_OAM_DMA_END,
    TRACE_LCD_CGB_DMA_START,
    TRACE_LCD_CGB_DMA_BLOCK,
    TRACE_LCD_CGB_DMA_END,
    TRACE_LCD_CGB_DMA_CANCEL,
};

enum GB_Trace_Input_Event : u8
{
    TRACE_INPUT_READ = 0,
    TRACE_INPUT_WRITE,
};

enum GB_Trace_Timer_Event : u8
{
    TRACE_TIMER_IRQ_REQUEST = 0,
    TRACE_TIMER_DIV_WRITE,
    TRACE_TIMER_TIMA_WRITE,
    TRACE_TIMER_TMA_WRITE,
    TRACE_TIMER_TAC_WRITE,
    TRACE_TIMER_RELOAD,
};

enum GB_Trace_APU_Event : u8
{
    TRACE_APU_GLOBAL_WRITE = 0,
    TRACE_APU_PULSE1_WRITE,
    TRACE_APU_PULSE2_WRITE,
    TRACE_APU_WAVE_WRITE,
    TRACE_APU_NOISE_WRITE,
    TRACE_APU_WAVE_RAM_WRITE,
};

enum GB_Trace_Serial_Event : u8
{
    TRACE_SERIAL_REG_WRITE = 0,
    TRACE_SERIAL_TRANSFER_START,
    TRACE_SERIAL_TRANSFER_END,
    TRACE_SERIAL_IRQ_REQUEST,
};

enum GB_Trace_Mapper_Event : u8
{
    TRACE_MAPPER_ROM = 0,
    TRACE_MAPPER_RAM_RTC,
    TRACE_MAPPER_CONTROL,
};

#define TRACE_EVENT_FLAG(event)              (1U << (event))

#define TRACE_LCD_FILTER_REGISTERS           TRACE_EVENT_FLAG(TRACE_LCD_REG_WRITE)
#define TRACE_LCD_FILTER_INTERRUPTS          \
    (TRACE_EVENT_FLAG(TRACE_LCD_VBLANK_IRQ) | TRACE_EVENT_FLAG(TRACE_LCD_STAT_IRQ))
#define TRACE_LCD_FILTER_DMA                  \
    (TRACE_EVENT_FLAG(TRACE_LCD_OAM_DMA_START) | TRACE_EVENT_FLAG(TRACE_LCD_OAM_DMA_END) | \
     TRACE_EVENT_FLAG(TRACE_LCD_CGB_DMA_START) | TRACE_EVENT_FLAG(TRACE_LCD_CGB_DMA_BLOCK) | \
     TRACE_EVENT_FLAG(TRACE_LCD_CGB_DMA_END) | TRACE_EVENT_FLAG(TRACE_LCD_CGB_DMA_CANCEL))
#define TRACE_LCD_FILTER_ALL                  \
    (TRACE_LCD_FILTER_REGISTERS | TRACE_LCD_FILTER_INTERRUPTS | TRACE_LCD_FILTER_DMA)

#define TRACE_INPUT_FILTER_READS             TRACE_EVENT_FLAG(TRACE_INPUT_READ)
#define TRACE_INPUT_FILTER_WRITES            TRACE_EVENT_FLAG(TRACE_INPUT_WRITE)
#define TRACE_INPUT_FILTER_ALL               (TRACE_INPUT_FILTER_READS | TRACE_INPUT_FILTER_WRITES)

#define TRACE_TIMER_FILTER_INTERRUPTS        TRACE_EVENT_FLAG(TRACE_TIMER_IRQ_REQUEST)
#define TRACE_TIMER_FILTER_REGISTERS         \
    (TRACE_EVENT_FLAG(TRACE_TIMER_DIV_WRITE) | TRACE_EVENT_FLAG(TRACE_TIMER_TIMA_WRITE) | \
     TRACE_EVENT_FLAG(TRACE_TIMER_TMA_WRITE) | TRACE_EVENT_FLAG(TRACE_TIMER_TAC_WRITE) | \
     TRACE_EVENT_FLAG(TRACE_TIMER_RELOAD))
#define TRACE_TIMER_FILTER_ALL               (TRACE_TIMER_FILTER_INTERRUPTS | TRACE_TIMER_FILTER_REGISTERS)

#define TRACE_APU_FILTER_GLOBAL              TRACE_EVENT_FLAG(TRACE_APU_GLOBAL_WRITE)
#define TRACE_APU_FILTER_PULSE1              TRACE_EVENT_FLAG(TRACE_APU_PULSE1_WRITE)
#define TRACE_APU_FILTER_PULSE2              TRACE_EVENT_FLAG(TRACE_APU_PULSE2_WRITE)
#define TRACE_APU_FILTER_WAVE                TRACE_EVENT_FLAG(TRACE_APU_WAVE_WRITE)
#define TRACE_APU_FILTER_NOISE               TRACE_EVENT_FLAG(TRACE_APU_NOISE_WRITE)
#define TRACE_APU_FILTER_WAVE_RAM            TRACE_EVENT_FLAG(TRACE_APU_WAVE_RAM_WRITE)
#define TRACE_APU_FILTER_ALL                 \
    (TRACE_APU_FILTER_GLOBAL | TRACE_APU_FILTER_PULSE1 | TRACE_APU_FILTER_PULSE2 | \
     TRACE_APU_FILTER_WAVE | TRACE_APU_FILTER_NOISE | TRACE_APU_FILTER_WAVE_RAM)

#define TRACE_SERIAL_FILTER_REGISTERS        TRACE_EVENT_FLAG(TRACE_SERIAL_REG_WRITE)
#define TRACE_SERIAL_FILTER_TRANSFERS        \
    (TRACE_EVENT_FLAG(TRACE_SERIAL_TRANSFER_START) | TRACE_EVENT_FLAG(TRACE_SERIAL_TRANSFER_END))
#define TRACE_SERIAL_FILTER_INTERRUPTS       TRACE_EVENT_FLAG(TRACE_SERIAL_IRQ_REQUEST)
#define TRACE_SERIAL_FILTER_ALL              \
    (TRACE_SERIAL_FILTER_REGISTERS | TRACE_SERIAL_FILTER_TRANSFERS | TRACE_SERIAL_FILTER_INTERRUPTS)

#define TRACE_MAPPER_FILTER_ROM              TRACE_EVENT_FLAG(TRACE_MAPPER_ROM)
#define TRACE_MAPPER_FILTER_RAM_RTC          TRACE_EVENT_FLAG(TRACE_MAPPER_RAM_RTC)
#define TRACE_MAPPER_FILTER_CONTROL          TRACE_EVENT_FLAG(TRACE_MAPPER_CONTROL)
#define TRACE_MAPPER_FILTER_ALL              \
    (TRACE_MAPPER_FILTER_ROM | TRACE_MAPPER_FILTER_RAM_RTC | TRACE_MAPPER_FILTER_CONTROL)

#define TRACE_MAPPER_FLAG_RAM_ENABLED        0x01
#define TRACE_MAPPER_FLAG_RTC_ENABLED        0x02
#define TRACE_MAPPER_FLAG_MODE               0x04
#define TRACE_MAPPER_FLAG_RUMBLE             0x08
#define TRACE_MAPPER_FLAG_LOCKED             0x10

struct GB_Trace_Entry
{
    GB_Trace_Type type;
    u64 cycle;
    union
    {
        struct
        {
            u16 pc;
            u16 bank;
            u16 af;
            u16 bc;
            u16 de;
            u16 hl;
            u16 sp;
            u8 size;
            u8 halt_bug;
            u8 opcodes[4];
            char name[64];
        } cpu;

        struct
        {
            u16 pc;
            u16 vector;
            u8 type;
        } irq;

        struct
        {
            u16 address;
            u16 value;
            u16 value2;
            u16 value3;
            u16 length;
            u16 line;
            u8 reg;
            u8 event;
            u8 raw;
            u8 mode;
        } lcd;

        struct
        {
            u8 value;
            u8 result;
            u8 select;
            u8 player;
            u8 event;
            u8 sgb_state;
        } input;

        struct
        {
            u16 divider;
            u8 counter;
            u8 reload;
            u8 control;
            u8 value;
            u8 event;
            u8 enabled;
        } timer;

        struct
        {
            u16 address;
            u8 value;
            u8 effective;
            u8 event;
        } apu;

        struct
        {
            u64 link_cycle;
            u64 request_cycle;
            u64 first_shift_cycle;
            u32 bit_cycles;
            u32 transfer_id;
            u16 address;
            u8 data;
            u8 control;
            u8 value;
            u8 event;
            u8 internal_clock;
            u8 fast_clock;
            u8 cgb;
            u8 double_speed;
            u8 outgoing_byte;
        } serial;

        struct
        {
            u16 address;
            u16 rom_bank0;
            u16 rom_bank1;
            s16 ram_bank;
            u8 value;
            u8 mapper;
            u8 event;
            u8 flags;
            u8 flags_valid;
        } mapper;
    };
};

static_assert(sizeof(GB_Trace_Entry) <= 104, "Trace entry exceeds memory budget");

class TraceLogger
{
public:
    TraceLogger(const u64* master_clock_cycles = NULL, const u64* link_cable_cycles = NULL);
    ~TraceLogger();
    void Reset();
    bool SetCapacity(u32 capacity);
    INLINE bool IsEnabled(GB_Trace_Type type) const;
    INLINE bool IsEventEnabled(GB_Trace_Type type, u8 event) const;
    INLINE void TraceLog(const GB_Trace_Entry& entry);
    void SetEnabledFlags(u32 flags);
    void SetEventFilter(GB_Trace_Type type, u32 filter);
    u32 GetEnabledFlags() const;
    u32 GetEventFilter(GB_Trace_Type type) const;
    const GB_Trace_Entry* GetBuffer() const;
    u32 GetCount() const;
    u32 GetCapacity() const;
    u32 GetPosition() const;
    u64 GetTotalLogged() const;
    u64 GetSequence() const;
    u64 GetLinkCableCycle() const;
    const GB_Trace_Entry& GetEntry(u32 index) const;

private:
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    void UpdateEnabled();
#endif
    GB_Trace_Entry* m_buffer;
    u32 m_position;
    u32 m_count;
    u32 m_capacity;
    u32 m_enabled_flags;
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    bool m_enabled;
#endif
    u32 m_event_filters[TRACE_TYPE_COUNT];
    u64 m_total_logged;
    u64 m_sequence;
    const u64* m_master_clock_cycles;
    const u64* m_link_cable_cycles;
};

INLINE bool TraceLogger::IsEnabled(GB_Trace_Type type) const
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (likely(!m_enabled))
        return false;

    return type < TRACE_TYPE_COUNT && (m_enabled_flags & (1U << type)) != 0;
#else
    UNUSED(type);
    return false;
#endif
}

INLINE bool TraceLogger::IsEventEnabled(GB_Trace_Type type, u8 event) const
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (likely(!m_enabled))
        return false;

    return type < TRACE_TYPE_COUNT && (m_enabled_flags & (1U << type)) != 0 &&
        event < 32 && (m_event_filters[type] & TRACE_EVENT_FLAG(event)) != 0;
#else
    UNUSED(type);
    UNUSED(event);
    return false;
#endif
}

INLINE void TraceLogger::TraceLog(const GB_Trace_Entry& entry)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    m_buffer[m_position] = entry;
    if (IsValidPointer(m_master_clock_cycles))
        m_buffer[m_position].cycle = *m_master_clock_cycles;
    m_position++;
    if (m_position == m_capacity)
        m_position = 0;
    if (m_count < m_capacity)
        m_count++;
    m_total_logged++;
    m_sequence++;
#else
    UNUSED(entry);
#endif
}

#endif /* TRACE_LOGGER_H */
