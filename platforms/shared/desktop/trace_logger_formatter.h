/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2026  Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#ifndef TRACE_LOGGER_FORMATTER_H
#define TRACE_LOGGER_FORMATTER_H

#include "TraceLogger.h"

#define GB_TRACE_FORMAT_BUFFER_SIZE 512

class Memory;

struct GB_Trace_Format_Options
{
    bool bank;
    bool registers;
    bool flags;
    bool bytes;
    bool cycles;
    bool previous_cycle_valid;
    u64 previous_cycle;
};

GB_Disassembler_Record* trace_log_get_cpu_record(Memory* memory, const GB_Trace_Entry& entry);
void trace_log_format_cpu_bytes(const GB_Trace_Entry& entry, char* buf, int buf_size);
void trace_log_format_cycle_prefix(const GB_Trace_Entry& entry, bool previous_cycle_valid,
    u64 previous_cycle, char* buf, int buf_size);
void trace_log_format_entry(Memory* memory, const GB_Trace_Entry& entry,
    const GB_Trace_Format_Options& options, char* buf, int buf_size);

#endif /* TRACE_LOGGER_FORMATTER_H */
