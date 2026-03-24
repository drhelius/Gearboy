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

#include "TraceLogger.h"
#include <string.h>

TraceLogger::TraceLogger()
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    m_buffer = new GB_Trace_Entry[TRACE_BUFFER_SIZE];
    memset(m_buffer, 0, sizeof(GB_Trace_Entry) * TRACE_BUFFER_SIZE);
#else
    m_buffer = NULL;
#endif
    m_position = 0;
    m_count = 0;
    m_enabled_flags = 0;
    m_total_logged = 0;
}

TraceLogger::~TraceLogger()
{
    SafeDeleteArray(m_buffer);
}

void TraceLogger::Reset()
{
    m_position = 0;
    m_count = 0;
    m_total_logged = 0;
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    if (m_buffer)
        memset(m_buffer, 0, sizeof(GB_Trace_Entry) * TRACE_BUFFER_SIZE);
#endif
}

void TraceLogger::SetEnabledFlags(u32 flags)
{
    m_enabled_flags = flags;
}

u32 TraceLogger::GetEnabledFlags() const
{
    return m_enabled_flags;
}

const GB_Trace_Entry* TraceLogger::GetBuffer() const
{
    return m_buffer;
}

u32 TraceLogger::GetCount() const
{
    return m_count;
}

u32 TraceLogger::GetPosition() const
{
    return m_position;
}

u64 TraceLogger::GetTotalLogged() const
{
    return m_total_logged;
}

const GB_Trace_Entry& TraceLogger::GetEntry(u32 index) const
{
    static const GB_Trace_Entry k_empty = {};
    if (!m_buffer)
        return k_empty;
    u32 actual;
    if (m_count < TRACE_BUFFER_SIZE)
        actual = index;
    else
        actual = (m_position + index) % TRACE_BUFFER_SIZE;
    return m_buffer[actual];
}
