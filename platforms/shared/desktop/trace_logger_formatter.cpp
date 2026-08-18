/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2026  Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#include "trace_logger_formatter.h"
#include "Memory.h"
#include "Cartridge.h"
#include <cstdio>
#include <cstring>

GB_Disassembler_Record* trace_log_get_cpu_record(Memory* memory, const GB_Trace_Entry& entry)
{
    if (entry.cpu.size > sizeof(entry.cpu.opcodes))
        return NULL;

    GB_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.cpu.pc, entry.cpu.bank);
    if (!IsValidPointer(record) || record->size != entry.cpu.size ||
        memcmp(record->opcodes, entry.cpu.opcodes, entry.cpu.size) != 0)
    {
        return NULL;
    }

    return record;
}

void trace_log_format_cpu_bytes(const GB_Trace_Entry& entry, char* buf, int buf_size)
{
    static const char k_hex[] = "0123456789ABCDEF";
    int pos = 0;
    u8 size = MIN(entry.cpu.size, (u8)sizeof(entry.cpu.opcodes));
    for (u8 i = 0; i < size && (pos + 3) < buf_size; i++)
    {
        u8 value = entry.cpu.opcodes[i];
        buf[pos++] = k_hex[value >> 4];
        buf[pos++] = k_hex[value & 0x0F];
        buf[pos++] = ' ';
    }
    buf[pos] = '\0';
}

void trace_log_format_cycle_prefix(const GB_Trace_Entry& entry, bool previous_cycle_valid,
    u64 previous_cycle, char* buf, int buf_size)
{
    if (previous_cycle_valid && entry.cycle >= previous_cycle)
    {
        snprintf(buf, buf_size, "@%012llu +%-12llu ",
                 (unsigned long long)entry.cycle,
                 (unsigned long long)(entry.cycle - previous_cycle));
    }
    else if (previous_cycle_valid)
    {
        snprintf(buf, buf_size, "@%012llu RESET         ",
                 (unsigned long long)entry.cycle);
    }
    else
    {
        snprintf(buf, buf_size, "@%012llu               ",
                 (unsigned long long)entry.cycle);
    }
}

static const char* get_lcd_register_name(u8 reg)
{
    switch (reg)
    {
        case 0x40: return "LCDC";
        case 0x41: return "STAT";
        case 0x42: return "SCY";
        case 0x43: return "SCX";
        case 0x45: return "LYC";
        case 0x46: return "DMA";
        case 0x47: return "BGP";
        case 0x48: return "OBP0";
        case 0x49: return "OBP1";
        case 0x4A: return "WY";
        case 0x4B: return "WX";
        case 0x4F: return "VBK";
        case 0x51: return "HDMA1";
        case 0x52: return "HDMA2";
        case 0x53: return "HDMA3";
        case 0x54: return "HDMA4";
        case 0x55: return "HDMA5";
        case 0x68: return "BCPS";
        case 0x69: return "BCPD";
        case 0x6A: return "OCPS";
        case 0x6B: return "OCPD";
        default: return "???";
    }
}

static const char* get_apu_register_name(u16 address)
{
    static const char* k_names[] = {
        "NR10", "NR11", "NR12", "NR13", "NR14", "???", "NR21", "NR22",
        "NR23", "NR24", "NR30", "NR31", "NR32", "NR33", "NR34", "???",
        "NR41", "NR42", "NR43", "NR44", "NR50", "NR51", "NR52"
    };

    if (address >= 0xFF10 && address <= 0xFF26)
        return k_names[address - 0xFF10];
    if (address >= 0xFF30 && address <= 0xFF3F)
        return "WAVE";
    return "???";
}

static const char* get_mapper_name(u8 mapper)
{
    static const char* k_names[] = {
        "ROM", "MBC1", "MBC2", "MBC3", "MBC5", "MBC1M", "HuC1", "HuC3",
        "MMM01", "CAMERA", "MBC7", "TAMA5", "WISDOM", "M161", "SACHEN1",
        "SACHEN2", "PKJD", "BUNG", "POKE2", "UNKNOWN"
    };
    if (mapper < (sizeof(k_names) / sizeof(k_names[0])))
        return k_names[mapper];
    return "UNKNOWN";
}

static void format_cpu_entry(Memory* memory, const GB_Trace_Entry& entry,
    const GB_Trace_Format_Options& options, char* buf, int buf_size)
{
    GB_Disassembler_Record* record = trace_log_get_cpu_record(memory, entry);
    char instr[64] = "???";
    char bytes[16] = "";

    if (IsValidPointer(record))
    {
        strncpy(instr, record->name, sizeof(instr) - 1);
        instr[sizeof(instr) - 1] = '\0';

        char* p = instr;
        while (*p)
        {
            if (*p == '{')
            {
                char* end = strchr(p, '}');
                if (end)
                    memmove(p, end + 1, strlen(end + 1) + 1);
                else
                    break;
            }
            else
                p++;
        }
    }

    trace_log_format_cpu_bytes(entry, bytes, sizeof(bytes));

    char bank[8] = "";
    if (options.bank)
        snprintf(bank, sizeof(bank), "%03X:", entry.cpu.bank);

    char registers[80] = "";
    if (options.registers)
    {
        snprintf(registers, sizeof(registers), "A:%02X  BC:%04X  DE:%04X  HL:%04X  SP:%04X  ",
                 (entry.cpu.af >> 8) & 0xFF, entry.cpu.bc, entry.cpu.de, entry.cpu.hl, entry.cpu.sp);
    }

    char flags[20] = "";
    if (options.flags)
    {
        u8 f = entry.cpu.af & 0xFF;
        snprintf(flags, sizeof(flags), "%c%c%c%c  ",
                 (f & FLAG_ZERO) ? 'Z' : 'z',
                 (f & FLAG_SUB) ? 'N' : 'n',
                 (f & FLAG_HALF) ? 'H' : 'h',
                 (f & FLAG_CARRY) ? 'C' : 'c');
    }

    snprintf(buf, buf_size, "%s%04X  %s%s%-24s %s%s",
             bank, entry.cpu.pc, registers, flags, instr,
             options.bytes ? bytes : "", entry.cpu.halt_bug ? "[HALT bug]" : "");
}

void trace_log_format_entry(Memory* memory, const GB_Trace_Entry& entry,
    const GB_Trace_Format_Options& options, char* buf, int buf_size)
{
    if (options.cycles)
    {
        GB_Trace_Format_Options body_options = options;
        body_options.cycles = false;
        char body[GB_TRACE_FORMAT_BUFFER_SIZE];
        char prefix[64];
        trace_log_format_entry(memory, entry, body_options, body, sizeof(body));
        trace_log_format_cycle_prefix(entry, options.previous_cycle_valid,
                                      options.previous_cycle, prefix, sizeof(prefix));
        snprintf(buf, buf_size, "%s%s", prefix, body);
        return;
    }

    switch (entry.type)
    {
        case TRACE_CPU:
            format_cpu_entry(memory, entry, options, buf, buf_size);
            break;
        case TRACE_CPU_IRQ:
        {
            static const char* k_irq_names[] = {"???", "VBlank", "LCDSTAT", "Timer", "Serial", "Joypad"};
            const char* name = (entry.irq.type >= 1 && entry.irq.type <= 5) ? k_irq_names[entry.irq.type] : "???";
            snprintf(buf, buf_size, "  [CPU]  %-8s  PC:$%04X  Vector:$%04X",
                     name, entry.irq.pc, entry.irq.vector);
            break;
        }
        case TRACE_LCD:
            switch (entry.lcd.event)
            {
                case TRACE_LCD_REG_WRITE:
                    if (entry.lcd.reg == 0x69 || entry.lcd.reg == 0x6B)
                    {
                        snprintf(buf, buf_size, "  [LCD]  %-5s    Raw:$%02X  Value:$%02X  Index:$%02X->$%02X  Line:%u Mode:%u",
                                 get_lcd_register_name(entry.lcd.reg), entry.lcd.raw, entry.lcd.value,
                                 entry.lcd.value2, entry.lcd.value3, entry.lcd.line, entry.lcd.mode);
                    }
                    else
                    {
                        snprintf(buf, buf_size, "  [LCD]  %-5s    Raw:$%02X  Value:$%02X  Line:%u Mode:%u",
                                 get_lcd_register_name(entry.lcd.reg), entry.lcd.raw, entry.lcd.value,
                                 entry.lcd.line, entry.lcd.mode);
                    }
                    break;
                case TRACE_LCD_VBLANK_IRQ:
                    snprintf(buf, buf_size, "  [LCD]  VBLANK IRQ  Line:%u Mode:%u", entry.lcd.line, entry.lcd.mode);
                    break;
                case TRACE_LCD_STAT_IRQ:
                    snprintf(buf, buf_size, "  [LCD]  STAT IRQ    Line:%u Mode:%u Source:$%02X",
                             entry.lcd.line, entry.lcd.mode, entry.lcd.value);
                    break;
                case TRACE_LCD_OAM_DMA_START:
                case TRACE_LCD_OAM_DMA_END:
                    snprintf(buf, buf_size, "  [LCD]  OAM DMA %s  Source:$%04X Dest:$%04X Length:$%04X",
                             entry.lcd.event == TRACE_LCD_OAM_DMA_START ? "START" : "END  ",
                             entry.lcd.address, entry.lcd.value, entry.lcd.length);
                    break;
                case TRACE_LCD_CGB_DMA_START:
                case TRACE_LCD_CGB_DMA_BLOCK:
                case TRACE_LCD_CGB_DMA_END:
                case TRACE_LCD_CGB_DMA_CANCEL:
                {
                    static const char* k_names[] = {"START", "BLOCK", "END", "CANCEL"};
                    const char* name = k_names[entry.lcd.event - TRACE_LCD_CGB_DMA_START];
                    snprintf(buf, buf_size, "  [LCD]  CGB DMA %-6s Source:$%04X Dest:$%04X Length:$%04X Line:%u",
                             name, entry.lcd.address, entry.lcd.value, entry.lcd.length, entry.lcd.line);
                    break;
                }
                default:
                    snprintf(buf, buf_size, "  [LCD]  ???");
                    break;
            }
            break;
        case TRACE_INPUT:
            snprintf(buf, buf_size, "  [INP]  %-5s    P1:$%02X Result:$%02X Select:$%02X Player:%u SGB:$%02X",
                     entry.input.event == TRACE_INPUT_READ ? "READ" : "WRITE",
                     entry.input.value, entry.input.result, entry.input.select,
                     entry.input.player + 1, entry.input.sgb_state);
            break;
        case TRACE_TIMER:
        {
            static const char* k_names[] = {"IRQ", "DIV WRITE", "TIMA WRITE", "TMA WRITE", "TAC WRITE", "RELOAD"};
            const char* name = entry.timer.event < 6 ? k_names[entry.timer.event] : "???";
            snprintf(buf, buf_size, "  [TIM]  %-10s DIV:$%04X TIMA:$%02X TMA:$%02X TAC:$%02X Value:$%02X Enabled:%u",
                     name, entry.timer.divider, entry.timer.counter, entry.timer.reload,
                     entry.timer.control, entry.timer.value, entry.timer.enabled);
            break;
        }
        case TRACE_APU:
            snprintf(buf, buf_size, "  [APU]  %-5s    Addr:$%04X Raw:$%02X Value:$%02X",
                     get_apu_register_name(entry.apu.address), entry.apu.address,
                     entry.apu.value, entry.apu.effective);
            break;
        case TRACE_SERIAL:
        {
            static const char* k_names[] = {"REG WRITE", "START", "END", "IRQ"};
            const char* name = entry.serial.event < 4 ? k_names[entry.serial.event] : "???";
            snprintf(buf, buf_size, "  [SER]  %-9s SB:$%02X SC:$%02X Value:$%02X Clock:%s",
                     name, entry.serial.data, entry.serial.control, entry.serial.value,
                     entry.serial.internal_clock ? "internal" : "external");
            break;
        }
        case TRACE_MAPPER:
        {
            static const char* k_names[] = {"ROM", "RAM/RTC", "CONTROL"};
            const char* name = entry.mapper.event < 3 ? k_names[entry.mapper.event] : "???";
            char state[96] = "";
            if (entry.mapper.flags_valid)
            {
                snprintf(state, sizeof(state), " ExtRAM:%s RTC:%s Mode:%u Rumble:%s Locked:%s",
                         (entry.mapper.flags & TRACE_MAPPER_FLAG_RAM_ENABLED) ? "on" : "off",
                         (entry.mapper.flags & TRACE_MAPPER_FLAG_RTC_ENABLED) ? "on" : "off",
                         (entry.mapper.flags & TRACE_MAPPER_FLAG_MODE) ? 1 : 0,
                         (entry.mapper.flags & TRACE_MAPPER_FLAG_RUMBLE) ? "on" : "off",
                         (entry.mapper.flags & TRACE_MAPPER_FLAG_LOCKED) ? "yes" : "no");
            }
            if (entry.mapper.ram_bank < 0)
            {
                snprintf(buf, buf_size, "  [MAP]  %-8s %-7s Addr:$%04X Value:$%02X ROM0:$%03X ROM1:$%03X RAM:RTC%s",
                         get_mapper_name(entry.mapper.mapper), name, entry.mapper.address, entry.mapper.value,
                         entry.mapper.rom_bank0, entry.mapper.rom_bank1, state);
            }
            else
            {
                snprintf(buf, buf_size, "  [MAP]  %-8s %-7s Addr:$%04X Value:$%02X ROM0:$%03X ROM1:$%03X RAM:%d%s",
                         get_mapper_name(entry.mapper.mapper), name, entry.mapper.address, entry.mapper.value,
                         entry.mapper.rom_bank0, entry.mapper.rom_bank1, entry.mapper.ram_bank, state);
            }
            break;
        }
        default:
            snprintf(buf, buf_size, "  [???]");
            break;
    }
}
