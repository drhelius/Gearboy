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

#ifndef LINK_CABLE_H
#define LINK_CABLE_H

#include "definitions.h"

#define LINK_CABLE_MAX_PEERS 2
#define LINK_CABLE_MAX_PROMISE_CYCLES 64
#define LINK_CABLE_IDLE_SYNC_CYCLES 32
#define LINK_CABLE_FAST_SYNC_CYCLES 8

struct GB_LinkCableTransfer
{
    u64 request_cycle;
    u64 first_shift_cycle;
    u32 bit_cycles;
    u32 transfer_id;
    u8 incoming_byte;
    u8 local_byte;
};

typedef void (*GB_LinkCableStateCallback)(u64 cycle, u8 sb, u8 sc,void* user_data);
typedef void (*GB_LinkCableStartCallback)(u64 request_cycle, u64 first_shift_cycle,
    u32 bit_cycles, u8 outgoing_byte, u32 transfer_id, u8* incoming_byte, void* user_data);
typedef bool (*GB_LinkCablePollCallback)(u64 current_cycle, GB_LinkCableTransfer* transfer, void* user_data);
typedef void (*GB_LinkCableSyncCallback)(u64 cycle, u32 promise_cycles, void* user_data);

#endif /* LINK_CABLE_H */
