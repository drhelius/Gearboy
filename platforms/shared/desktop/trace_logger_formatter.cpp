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
#include "Cartridge.h"
#include "common.h"
#include <cstdio>
#include <cstring>

void trace_log_format_cycle_prefix(const GB_Trace_Entry& entry, bool previous_cycle_valid,
    u64 previous_cycle, char* buffer, size_t buffer_size)
{
    if (previous_cycle_valid && entry.cycle >= previous_cycle)
    {
        snprintf(buffer, buffer_size, "@%012llu +%-12llu ",
                 (unsigned long long)entry.cycle,
                 (unsigned long long)(entry.cycle - previous_cycle));
    }
    else if (previous_cycle_valid)
    {
        snprintf(buffer, buffer_size, "@%012llu RESET         ",
                 (unsigned long long)entry.cycle);
    }
    else
    {
        snprintf(buffer, buffer_size, "@%012llu               ",
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
        default: return NULL;
    }
}

static const char* get_lcd_mode_name(u8 mode)
{
    switch (mode)
    {
        case 0: return "HBLANK";
        case 1: return "VBLANK";
        case 2: return "OAM";
        case 3: return "TRANSFER";
        default: return "UNKNOWN";
    }
}

static const char* get_apu_register_name(u16 address)
{
    static const char* k_names[] = {
        "NR10", "NR11", "NR12", "NR13", "NR14", NULL, "NR21", "NR22",
        "NR23", "NR24", "NR30", "NR31", "NR32", "NR33", "NR34", NULL,
        "NR41", "NR42", "NR43", "NR44", "NR50", "NR51", "NR52"
    };

    if (address >= 0xFF10 && address <= 0xFF26)
        return k_names[address - 0xFF10];
    if (address >= 0xFF30 && address <= 0xFF3F)
        return "WAVE";
    return NULL;
}

static const char* get_mapper_name(u8 mapper)
{
    static const char* k_names[] = {
        "ROM", "MBC1", "MBC2", "MBC3", "MBC5", "MBC1M", "HuC1", "HuC3",
        "MMM01", "CAMERA", "MBC7", "TAMA5", "WISDOM", "M161", "SACHEN1",
        "SACHEN2", "PKJD", "BUNG", "POKE2", "UNKNOWN"
    };
    if (mapper < (sizeof(k_names) / sizeof(k_names[0])) - 1)
        return k_names[mapper];
    return NULL;
}

static void append_list_item(char* buffer, size_t buffer_size,
    const char* item, bool* first)
{
    size_t length = strlen(buffer);
    if (length >= buffer_size)
        return;

    snprintf(buffer + length, buffer_size - length, "%s%s",
             *first ? "" : "|", item);
    *first = false;
}

static void format_stat_sources(u8 source, char* buffer, size_t buffer_size)
{
    bool first = true;
    buffer[0] = '\0';

    if (source & 0x01) append_list_item(buffer, buffer_size, "HBLANK", &first);
    if (source & 0x02) append_list_item(buffer, buffer_size, "VBLANK", &first);
    if (source & 0x04) append_list_item(buffer, buffer_size, "OAM", &first);
    if (source & 0x08) append_list_item(buffer, buffer_size, "LYC", &first);
    if (first) append_list_item(buffer, buffer_size, "NONE", &first);
}

static void format_lcd_register_details(u8 reg, u8 raw, u8 value,
    char* buffer, size_t buffer_size)
{
    buffer[0] = '\0';

    switch (reg)
    {
        case 0x40:
            snprintf(buffer, buffer_size, " LCD:%s BG/Priority:%s OBJ:%s WIN:%s",
                     (value & 0x80) ? "on" : "off",
                     (value & 0x01) ? "on" : "off",
                     (value & 0x02) ? "on" : "off",
                     (value & 0x20) ? "on" : "off");
            break;
        case 0x41:
        {
            char sources[48];
            format_stat_sources((value >> 3) & 0x0F, sources, sizeof(sources));
            snprintf(buffer, buffer_size, " IRQ:%s LYC:%s",
                     sources, (value & 0x04) ? "match" : "different");
            break;
        }
        case 0x46:
            snprintf(buffer, buffer_size, " Source:$%04X", value << 8);
            break;
        case 0x47:
        case 0x48:
        case 0x49:
            snprintf(buffer, buffer_size, " Map:0>%u,1>%u,2>%u,3>%u",
                     value & 0x03, (value >> 2) & 0x03,
                     (value >> 4) & 0x03, (value >> 6) & 0x03);
            break;
        case 0x4F:
            snprintf(buffer, buffer_size, " Bank:$%X", value & 0x01);
            break;
        case 0x55:
            snprintf(buffer, buffer_size, " DMA:%s Blocks:$%02X",
                     (raw & 0x80) ? "HDMA" : "GDMA", (raw & 0x7F) + 1);
            break;
        case 0x68:
        case 0x6A:
            snprintf(buffer, buffer_size, " Index:$%02X Auto:%s",
                     value & 0x3F, (value & 0x80) ? "on" : "off");
            break;
        default:
            break;
    }
}

static const char* get_input_select_name(u8 select)
{
    switch (select & 0x30)
    {
        case 0x00: return "BOTH";
        case 0x10: return "BUTTONS";
        case 0x20: return "DIRECTION";
        default: return "NONE";
    }
}

static void format_input_pressed(u8 result, u8 select,
    char* buffer, size_t buffer_size)
{
    static const char* k_button_names[] = {"A", "B", "SELECT", "START"};
    static const char* k_direction_names[] = {"RIGHT", "LEFT", "UP", "DOWN"};
    static const char* k_both_names[] = {"A/RIGHT", "B/LEFT", "SELECT/UP", "START/DOWN"};
    const char** names;
    bool first = true;
    buffer[0] = '\0';

    switch (select & 0x30)
    {
        case 0x10: names = k_button_names; break;
        case 0x20: names = k_direction_names; break;
        case 0x00: names = k_both_names; break;
        default: return;
    }

    for (int bit = 0; bit < 4; bit++)
    {
        if ((result & (1U << bit)) == 0)
            append_list_item(buffer, buffer_size, names[bit], &first);
    }
}

static void format_sgb_state(u8 state, char* buffer, size_t buffer_size)
{
    bool first = true;
    buffer[0] = '\0';

    if ((state & 0x01) == 0)
    {
        snprintf(buffer, buffer_size, "none");
        return;
    }

    if (state & 0x02) append_list_item(buffer, buffer_size, "MULTI", &first);
    if (state & 0x04) append_list_item(buffer, buffer_size, "PULSE", &first);
    if (state & 0x08) append_list_item(buffer, buffer_size, "WRITE", &first);
    if (state & 0x10) append_list_item(buffer, buffer_size, "STOP", &first);
    if (state & 0x20) append_list_item(buffer, buffer_size, "DISABLED", &first);
    if (first) append_list_item(buffer, buffer_size, "READY", &first);
}

static const char* get_timer_clock_name(u8 control)
{
    static const char* k_clock_names[] = {"4096Hz", "262144Hz", "65536Hz", "16384Hz"};
    return k_clock_names[control & 0x03];
}

static void format_timer_control(u8 control, char* buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size, "$%02X(%s,%s)", control,
             (control & 0x04) ? "on" : "off", get_timer_clock_name(control));
}

static void format_apu_details(u16 address, u8 raw, u8 effective,
    char* buffer, size_t buffer_size)
{
    static const char* k_duty_names[] = {"12.5%", "25%", "50%", "75%"};
    static const char* k_level_names[] = {"mute", "100%", "50%", "25%"};
    buffer[0] = '\0';

    switch (address)
    {
        case 0xFF10:
            snprintf(buffer, buffer_size, " Sweep:%u,%s,Shift:%u",
                     (raw >> 4) & 0x07, (raw & 0x08) ? "down" : "up", raw & 0x07);
            break;
        case 0xFF11:
        case 0xFF16:
            snprintf(buffer, buffer_size, " Duty:%s Length:%u",
                     k_duty_names[(raw >> 6) & 0x03], 64 - (raw & 0x3F));
            break;
        case 0xFF12:
        case 0xFF17:
        case 0xFF21:
            snprintf(buffer, buffer_size, " Volume:$%X Env:%s/%u DAC:%s",
                     (raw >> 4) & 0x0F, (raw & 0x08) ? "up" : "down", raw & 0x07,
                     (raw & 0xF8) ? "on" : "off");
            break;
        case 0xFF14:
        case 0xFF19:
        case 0xFF1E:
            snprintf(buffer, buffer_size, " FreqHi:$%X Length:%s Trigger:%s",
                     raw & 0x07, (raw & 0x40) ? "on" : "off",
                     (raw & 0x80) ? "yes" : "no");
            break;
        case 0xFF1A:
            snprintf(buffer, buffer_size, " DAC:%s", (raw & 0x80) ? "on" : "off");
            break;
        case 0xFF1B:
            snprintf(buffer, buffer_size, " Length:%u", 256 - raw);
            break;
        case 0xFF1C:
            snprintf(buffer, buffer_size, " Level:%s", k_level_names[(raw >> 5) & 0x03]);
            break;
        case 0xFF20:
            snprintf(buffer, buffer_size, " Length:%u", 64 - (raw & 0x3F));
            break;
        case 0xFF22:
            snprintf(buffer, buffer_size, " Shift:%u Width:%u-bit Div:$%X",
                     (raw >> 4) & 0x0F, (raw & 0x08) ? 7 : 15, raw & 0x07);
            break;
        case 0xFF23:
            snprintf(buffer, buffer_size, " Length:%s Trigger:%s",
                     (raw & 0x40) ? "on" : "off", (raw & 0x80) ? "yes" : "no");
            break;
        case 0xFF24:
            snprintf(buffer, buffer_size, " Left:$%X Right:$%X VIN-L:%s VIN-R:%s",
                     (raw >> 4) & 0x07, raw & 0x07,
                     (raw & 0x80) ? "on" : "off", (raw & 0x08) ? "on" : "off");
            break;
        case 0xFF25:
            snprintf(buffer, buffer_size, " Route-L:$%X Route-R:$%X",
                     (raw >> 4) & 0x0F, raw & 0x0F);
            break;
        case 0xFF26:
            snprintf(buffer, buffer_size, " Power:%s Active:$%X",
                     (raw & 0x80) ? "on" : "off", effective & 0x0F);
            break;
        default:
            if (address >= 0xFF30 && address <= 0xFF3F)
                snprintf(buffer, buffer_size, " Index:$%X", address - 0xFF30);
            break;
    }
}

static void format_mapper_state(u8 flags, char* buffer, size_t buffer_size)
{
    bool first = true;
    buffer[0] = '\0';

    if (flags & TRACE_MAPPER_FLAG_RAM_ENABLED) append_list_item(buffer, buffer_size, "RAM", &first);
    if (flags & TRACE_MAPPER_FLAG_RTC_ENABLED) append_list_item(buffer, buffer_size, "RTC", &first);
    if (flags & TRACE_MAPPER_FLAG_MODE) append_list_item(buffer, buffer_size, "MODE", &first);
    if (flags & TRACE_MAPPER_FLAG_RUMBLE) append_list_item(buffer, buffer_size, "RUMBLE", &first);
    if (flags & TRACE_MAPPER_FLAG_LOCKED) append_list_item(buffer, buffer_size, "LOCKED", &first);
    if (first) append_list_item(buffer, buffer_size, "none", &first);
}

static void format_cpu_entry(const GB_Trace_Entry& entry,
    const GB_Trace_Format_Options& options, char* buf, int buf_size)
{
    char instr[64] = "???";
    char bytes[16] = "";

    if (entry.cpu.name[0] != 0)
    {
        strncpy(instr, entry.cpu.name, sizeof(instr) - 1);
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

    format_hex_bytes(entry.cpu.opcodes,
                     MIN(entry.cpu.size, (u8)sizeof(entry.cpu.opcodes)),
                     bytes, sizeof(bytes));

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

void trace_logger_format_entry(const GB_Trace_Entry& entry,
    const GB_Trace_Format_Options& options, char* buffer, size_t buffer_size)
{
    char* buf = buffer;
    int buf_size = (int)buffer_size;

    if (options.cycles)
    {
        GB_Trace_Format_Options body_options = options;
        body_options.cycles = false;
        char body[GB_TRACE_FORMAT_BUFFER_SIZE];
        char prefix[64];
        trace_logger_format_entry(entry, body_options, body, sizeof(body));
        trace_log_format_cycle_prefix(entry, options.previous_cycle_valid,
                                      options.previous_cycle, prefix, sizeof(prefix));
        snprintf(buffer, buffer_size, "%s%s", prefix, body);
        return;
    }

    switch (entry.type)
    {
        case TRACE_CPU:
            format_cpu_entry(entry, options, buffer, (int)buffer_size);
            break;
        case TRACE_CPU_IRQ:
        {
            static const char* k_irq_names[] = {"", "VBLANK", "LCDSTAT", "TIMER", "SERIAL", "JOYPAD"};
            if (entry.irq.type >= 1 && entry.irq.type <= 5)
            {
                snprintf(buf, buf_size, "  [CPU]  IRQ %-7s PC:$%04X Vector:$%04X",
                         k_irq_names[entry.irq.type], entry.irq.pc, entry.irq.vector);
            }
            else
            {
                snprintf(buf, buf_size, "  [CPU]  IRQ UNKNOWN($%02X) PC:$%04X Vector:$%04X",
                         entry.irq.type, entry.irq.pc, entry.irq.vector);
            }
            break;
        }
        case TRACE_LCD:
            switch (entry.lcd.event)
            {
                case TRACE_LCD_REG_WRITE:
                {
                    const char* name = get_lcd_register_name(entry.lcd.reg);
                    char mode[32];
                    char details[128];
                    snprintf(mode, sizeof(mode), "%s($%02X)",
                             get_lcd_mode_name(entry.lcd.mode), entry.lcd.mode);
                    format_lcd_register_details(entry.lcd.reg, entry.lcd.raw,
                                                (u8)entry.lcd.value,
                                                details, sizeof(details));
                    if (entry.lcd.reg == 0x69 || entry.lcd.reg == 0x6B)
                    {
                        if (name)
                        {
                            snprintf(buf, buf_size, "  [LCD]  WRITE %s($%04X) Raw:$%02X Value:$%02X Index:$%02X->$%02X LY:$%02X Mode:%s%s",
                                     name, entry.lcd.address, entry.lcd.raw, entry.lcd.value,
                                     entry.lcd.value2, entry.lcd.value3, entry.lcd.line, mode, details);
                        }
                        else
                        {
                            snprintf(buf, buf_size, "  [LCD]  WRITE REG($%04X) Raw:$%02X Value:$%02X Index:$%02X->$%02X LY:$%02X Mode:%s%s",
                                     entry.lcd.address, entry.lcd.raw, entry.lcd.value,
                                     entry.lcd.value2, entry.lcd.value3, entry.lcd.line, mode, details);
                        }
                    }
                    else
                    {
                        if (name)
                        {
                            snprintf(buf, buf_size, "  [LCD]  WRITE %s($%04X) Raw:$%02X Value:$%02X LY:$%02X Mode:%s%s",
                                     name, entry.lcd.address, entry.lcd.raw, entry.lcd.value,
                                     entry.lcd.line, mode, details);
                        }
                        else
                        {
                            snprintf(buf, buf_size, "  [LCD]  WRITE REG($%04X) Raw:$%02X Value:$%02X LY:$%02X Mode:%s%s",
                                     entry.lcd.address, entry.lcd.raw, entry.lcd.value,
                                     entry.lcd.line, mode, details);
                        }
                    }
                    break;
                }
                case TRACE_LCD_VBLANK_IRQ:
                    snprintf(buf, buf_size, "  [LCD]  VBLANK IRQ LY:$%02X Mode:%s($%02X)",
                             entry.lcd.line, get_lcd_mode_name(entry.lcd.mode), entry.lcd.mode);
                    break;
                case TRACE_LCD_STAT_IRQ:
                {
                    char sources[48];
                    format_stat_sources((u8)entry.lcd.value, sources, sizeof(sources));
                    snprintf(buf, buf_size, "  [LCD]  STAT IRQ Source:%s($%02X) LY:$%02X Mode:%s($%02X)",
                             sources, entry.lcd.value, entry.lcd.line,
                             get_lcd_mode_name(entry.lcd.mode), entry.lcd.mode);
                    break;
                }
                case TRACE_LCD_OAM_DMA_START:
                case TRACE_LCD_OAM_DMA_END:
                    snprintf(buf, buf_size, "  [LCD]  OAM DMA %-5s Src:$%04X Dst:$%04X Len:$%04X LY:$%02X Mode:%s($%02X)",
                             entry.lcd.event == TRACE_LCD_OAM_DMA_START ? "START" : "END",
                             entry.lcd.address, entry.lcd.value, entry.lcd.length,
                             entry.lcd.line, get_lcd_mode_name(entry.lcd.mode), entry.lcd.mode);
                    break;
                case TRACE_LCD_CGB_DMA_START:
                case TRACE_LCD_CGB_DMA_BLOCK:
                case TRACE_LCD_CGB_DMA_END:
                case TRACE_LCD_CGB_DMA_CANCEL:
                {
                    static const char* k_names[] = {"START", "BLOCK", "END", "CANCEL"};
                    const char* name = k_names[entry.lcd.event - TRACE_LCD_CGB_DMA_START];
                    snprintf(buf, buf_size, "  [LCD]  CGB DMA %-6s Src:$%04X Dst:$%04X Len:$%04X LY:$%02X Mode:%s($%02X)",
                             name, entry.lcd.address, entry.lcd.value, entry.lcd.length,
                             entry.lcd.line, get_lcd_mode_name(entry.lcd.mode), entry.lcd.mode);
                    break;
                }
                default:
                    snprintf(buf, buf_size, "  [LCD]  UNKNOWN EVENT($%02X)", entry.lcd.event);
                    break;
            }
            break;
        case TRACE_INPUT:
        {
            char pressed[80];
            char sgb[80];
            format_input_pressed(entry.input.result, entry.input.select, pressed, sizeof(pressed));
            format_sgb_state(entry.input.sgb_state, sgb, sizeof(sgb));
            if (entry.input.event == TRACE_INPUT_READ || entry.input.event == TRACE_INPUT_WRITE)
            {
                snprintf(buf, buf_size, "  [INP]  %-5s P1:$%02X Result:$%02X Select:%s($%02X) Pressed:%s Player:%u SGB:$%02X[%s]",
                         entry.input.event == TRACE_INPUT_READ ? "READ" : "WRITE",
                         entry.input.value, entry.input.result,
                         get_input_select_name(entry.input.select), entry.input.select,
                         pressed[0] != '\0' ? pressed : "none", entry.input.player + 1,
                         entry.input.sgb_state, sgb);
            }
            else
            {
                snprintf(buf, buf_size, "  [INP]  UNKNOWN EVENT($%02X) P1:$%02X Result:$%02X",
                         entry.input.event, entry.input.value, entry.input.result);
            }
            break;
        }
        case TRACE_TIMER:
        {
            char control[48];
            format_timer_control(entry.timer.control, control, sizeof(control));
            switch (entry.timer.event)
            {
                case TRACE_TIMER_IRQ_REQUEST:
                    snprintf(buf, buf_size, "  [TIM]  IRQ REQUEST TIMA:$%02X TMA:$%02X DIV:$%04X TAC:%s",
                             entry.timer.counter, entry.timer.reload, entry.timer.divider, control);
                    break;
                case TRACE_TIMER_DIV_WRITE:
                    snprintf(buf, buf_size, "  [TIM]  DIV WRITE Data:$%02X DIV:$%04X TIMA:$%02X TAC:%s",
                             entry.timer.value, entry.timer.divider, entry.timer.counter, control);
                    break;
                case TRACE_TIMER_TIMA_WRITE:
                    snprintf(buf, buf_size, "  [TIM]  TIMA WRITE Data:$%02X TIMA:$%02X TMA:$%02X DIV:$%04X TAC:%s",
                             entry.timer.value, entry.timer.counter, entry.timer.reload,
                             entry.timer.divider, control);
                    break;
                case TRACE_TIMER_TMA_WRITE:
                    snprintf(buf, buf_size, "  [TIM]  TMA WRITE Data:$%02X TMA:$%02X TIMA:$%02X DIV:$%04X TAC:%s",
                             entry.timer.value, entry.timer.reload, entry.timer.counter,
                             entry.timer.divider, control);
                    break;
                case TRACE_TIMER_TAC_WRITE:
                    snprintf(buf, buf_size, "  [TIM]  TAC WRITE Data:$%02X TAC:%s DIV:$%04X TIMA:$%02X TMA:$%02X",
                             entry.timer.value, control, entry.timer.divider,
                             entry.timer.counter, entry.timer.reload);
                    break;
                case TRACE_TIMER_RELOAD:
                    snprintf(buf, buf_size, "  [TIM]  TIMA RELOAD TIMA:$%02X TMA:$%02X DIV:$%04X TAC:%s",
                             entry.timer.counter, entry.timer.reload, entry.timer.divider, control);
                    break;
                default:
                    snprintf(buf, buf_size, "  [TIM]  UNKNOWN EVENT($%02X) DIV:$%04X TIMA:$%02X TMA:$%02X TAC:%s",
                             entry.timer.event, entry.timer.divider, entry.timer.counter,
                             entry.timer.reload, control);
                    break;
            }
            break;
        }
        case TRACE_APU:
        {
            const char* name = get_apu_register_name(entry.apu.address);
            char details[128];
            if (entry.apu.event > TRACE_APU_WAVE_RAM_WRITE)
            {
                snprintf(buf, buf_size, "  [APU]  UNKNOWN EVENT($%02X) Addr:$%04X Raw:$%02X Read:$%02X",
                         entry.apu.event, entry.apu.address,
                         entry.apu.value, entry.apu.effective);
                break;
            }
            format_apu_details(entry.apu.address, entry.apu.value, entry.apu.effective,
                               details, sizeof(details));
            if (name)
            {
                snprintf(buf, buf_size, "  [APU]  WRITE %s($%04X) Raw:$%02X Read:$%02X%s",
                         name, entry.apu.address, entry.apu.value, entry.apu.effective, details);
            }
            else
            {
                snprintf(buf, buf_size, "  [APU]  WRITE REG($%04X) Raw:$%02X Read:$%02X%s",
                         entry.apu.address, entry.apu.value, entry.apu.effective, details);
            }
            break;
        }
        case TRACE_SERIAL:
        {
            const char* clock = entry.serial.internal_clock ? "INTERNAL" : "EXTERNAL";
            const char* speed = (entry.serial.control & 0x02) ? "FAST" : "NORMAL";
            const char* state = (entry.serial.control & 0x80) ? "ACTIVE" : "IDLE";
            switch (entry.serial.event)
            {
                case TRACE_SERIAL_REG_WRITE:
                    snprintf(buf, buf_size, "  [SER]  REG WRITE Data:$%02X SB:$%02X SC:$%02X Clock:%s SpeedBit:%s State:%s",
                             entry.serial.value, entry.serial.data, entry.serial.control,
                             clock, speed, state);
                    break;
                case TRACE_SERIAL_TRANSFER_START:
                    snprintf(buf, buf_size, "  [SER]  TRANSFER START SB:$%02X SC:$%02X Clock:%s SpeedBit:%s",
                             entry.serial.data, entry.serial.control, clock, speed);
                    break;
                case TRACE_SERIAL_TRANSFER_END:
                    snprintf(buf, buf_size, "  [SER]  TRANSFER END SB:$%02X SC:$%02X Clock:%s SpeedBit:%s",
                             entry.serial.data, entry.serial.control, clock, speed);
                    break;
                case TRACE_SERIAL_IRQ_REQUEST:
                    snprintf(buf, buf_size, "  [SER]  IRQ REQUEST SB:$%02X SC:$%02X Clock:%s",
                             entry.serial.data, entry.serial.control, clock);
                    break;
                default:
                    snprintf(buf, buf_size, "  [SER]  UNKNOWN EVENT($%02X) SB:$%02X SC:$%02X Data:$%02X",
                             entry.serial.event, entry.serial.data,
                             entry.serial.control, entry.serial.value);
                    break;
            }
            break;
        }
        case TRACE_MAPPER:
        {
            const char* mapper = get_mapper_name(entry.mapper.mapper);
            char mapper_name[32];
            char state_names[64];
            char state[96] = "";
            if (mapper)
                snprintf(mapper_name, sizeof(mapper_name), "%s", mapper);
            else
                snprintf(mapper_name, sizeof(mapper_name), "UNKNOWN($%02X)", entry.mapper.mapper);

            if (entry.mapper.flags_valid)
            {
                format_mapper_state(entry.mapper.flags, state_names, sizeof(state_names));
                snprintf(state, sizeof(state), " State:$%02X[%s]", entry.mapper.flags, state_names);
            }

            switch (entry.mapper.event)
            {
                case TRACE_MAPPER_ROM:
                    snprintf(buf, buf_size, "  [MAP]  %s ROM WRITE Addr:$%04X Data:$%02X ROM0:$%03X ROMX:$%03X%s",
                             mapper_name, entry.mapper.address, entry.mapper.value,
                             entry.mapper.rom_bank0, entry.mapper.rom_bank1, state);
                    break;
                case TRACE_MAPPER_RAM_RTC:
                    if (entry.mapper.ram_bank < 0)
                    {
                        snprintf(buf, buf_size, "  [MAP]  %s RAM/RTC WRITE Addr:$%04X Data:$%02X RAM:RTC%s",
                                 mapper_name, entry.mapper.address, entry.mapper.value, state);
                    }
                    else
                    {
                        snprintf(buf, buf_size, "  [MAP]  %s RAM/RTC WRITE Addr:$%04X Data:$%02X RAM:$%02X%s",
                                 mapper_name, entry.mapper.address, entry.mapper.value,
                                 (u16)entry.mapper.ram_bank, state);
                    }
                    break;
                case TRACE_MAPPER_CONTROL:
                    if (entry.mapper.ram_bank < 0)
                    {
                        snprintf(buf, buf_size, "  [MAP]  %s CONTROL WRITE Addr:$%04X Data:$%02X ROM0:$%03X ROMX:$%03X RAM:RTC%s",
                                 mapper_name, entry.mapper.address, entry.mapper.value,
                                 entry.mapper.rom_bank0, entry.mapper.rom_bank1, state);
                    }
                    else
                    {
                        snprintf(buf, buf_size, "  [MAP]  %s CONTROL WRITE Addr:$%04X Data:$%02X ROM0:$%03X ROMX:$%03X RAM:$%02X%s",
                                 mapper_name, entry.mapper.address, entry.mapper.value,
                                 entry.mapper.rom_bank0, entry.mapper.rom_bank1,
                                 (u16)entry.mapper.ram_bank, state);
                    }
                    break;
                default:
                    snprintf(buf, buf_size, "  [MAP]  %s UNKNOWN EVENT($%02X) Addr:$%04X Data:$%02X",
                             mapper_name, entry.mapper.event,
                             entry.mapper.address, entry.mapper.value);
                    break;
            }
            break;
        }
        default:
            snprintf(buf, buf_size, "  [???]  TYPE:$%02X", entry.type);
            break;
    }
}
