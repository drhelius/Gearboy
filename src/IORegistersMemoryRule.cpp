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

#include "IORegistersMemoryRule.h"

IORegistersMemoryRule::IORegistersMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput, Audio* pAudio)
{
    m_pProcessor = pProcessor;
    m_pMemory = pMemory;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pAudio = pAudio;
    InitPointer(m_pSGB);
    InitPointer(m_pTraceLogger);
    m_bCGB = false;
}

IORegistersMemoryRule::~IORegistersMemoryRule()
{
}

void IORegistersMemoryRule::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void IORegistersMemoryRule::LogTraceInputEvent(u8 event, u8 value, u8 result)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (event == TRACE_INPUT_WRITE)
        result = m_pInput->Read();

    GB_Trace_Entry e = {};
    e.type = TRACE_INPUT;
    e.input.value = value;
    e.input.result = result;
    e.input.select = result & 0x30;
    e.input.event = event;

    if (IsValidPointer(m_pSGB))
    {
        e.input.player = (u8)m_pSGB->GetCurrentPlayer();
        e.input.sgb_state = 0x01;
        if (m_pSGB->GetPlayerCount() > 1)
            e.input.sgb_state |= 0x02;
        if (m_pSGB->IsReadyForPulse())
            e.input.sgb_state |= 0x04;
        if (m_pSGB->IsReadyForWrite())
            e.input.sgb_state |= 0x08;
        if (m_pSGB->IsReadyForStop())
            e.input.sgb_state |= 0x10;
        if (m_pSGB->AreCommandsDisabled())
            e.input.sgb_state |= 0x20;
    }

    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(value);
    UNUSED(result);
#endif
}

void IORegistersMemoryRule::LogTraceTimerEvent(u8 event, u8 value)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_TIMER;
    e.timer.divider = m_pProcessor->GetDIVCounter();
    e.timer.counter = m_pMemory->Retrieve(0xFF05);
    e.timer.reload = m_pMemory->Retrieve(0xFF06);
    e.timer.control = m_pMemory->Retrieve(0xFF07);
    e.timer.value = value;
    e.timer.event = event;
    e.timer.enabled = (e.timer.control & 0x04) ? 1 : 0;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(value);
#endif
}

void IORegistersMemoryRule::LogTraceSerialEvent(u8 event, u8 value)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_SERIAL;
    e.serial.data = m_pMemory->Retrieve(0xFF01);
    e.serial.control = m_pMemory->Retrieve(0xFF02);
    e.serial.value = value;
    e.serial.event = event;
    e.serial.internal_clock = (e.serial.control & 0x01) ? 1 : 0;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(value);
#endif
}

void IORegistersMemoryRule::LogTraceLCDRegister(u16 address, u8 raw)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    u8 effective;
    if (m_bCGB && address >= 0xFF51 && address <= 0xFF55)
        effective = m_pMemory->GetHDMARegister(address - 0xFF50);
    else
        effective = m_pMemory->Retrieve(address);

    u16 value2 = 0;
    u16 value3 = 0;
    if (address == 0xFF69 || address == 0xFF6B)
    {
        u16 palette_address = address == 0xFF69 ? 0xFF68 : 0xFF6A;
        u8 palette_index = m_pMemory->Retrieve(palette_address);
        value2 = palette_index;
        value3 = palette_index;
        if (m_bCGB && IsSetBit(palette_index, 7))
            value2 = (palette_index & 0x80) | ((palette_index - 1) & 0x3F);
    }

    GB_Trace_Entry e = {};
    e.type = TRACE_LCD;
    e.lcd.address = address;
    e.lcd.value = effective;
    e.lcd.line = m_pMemory->Retrieve(0xFF44);
    e.lcd.reg = (u8)(address & 0xFF);
    e.lcd.event = TRACE_LCD_REG_WRITE;
    e.lcd.raw = raw;
    e.lcd.mode = (u8)m_pVideo->GetCurrentStatusMode();
    e.lcd.value2 = value2;
    e.lcd.value3 = value3;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(address);
    UNUSED(raw);
#endif
}

void IORegistersMemoryRule::LogTraceLCDInterrupt(u8 event, u8 source)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_LCD;
    e.lcd.event = event;
    e.lcd.value = source;
    e.lcd.line = m_pMemory->Retrieve(0xFF44);
    e.lcd.mode = (u8)m_pVideo->GetCurrentStatusMode();
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(source);
#endif
}

void IORegistersMemoryRule::LogTraceAPURegister(u8 event, u16 address, u8 raw)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_APU;
    e.apu.address = address;
    e.apu.value = raw;
    e.apu.effective = m_pAudio->ReadAudioRegister(address);
    e.apu.event = event;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(address);
    UNUSED(raw);
#endif
}

void IORegistersMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}

void IORegistersMemoryRule::SetSGB(SGB* pSGB)
{
    m_pSGB = pSGB;
}
