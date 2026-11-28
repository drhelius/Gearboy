/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez
 * Copyright (C) 2026  Nolan Baker

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

#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "link_cable.h"

enum GB_PS2KeyboardMode
{
    GB_PS2KeyboardMode_Auto,
    GB_PS2KeyboardMode_Disabled,
    GB_PS2KeyboardMode_Enabled
};

class PS2Keyboard
{
public:
    static const int k_queue_size = 32;
    static const u32 k_bit_cycles = 512;

    PS2Keyboard();
    void Reset();
    bool QueueScanCode(u8 scan_code);
    static void StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent event, void* user_data);
    static void StartCallback(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles, u8 outgoing_byte,
        u32 transfer_id, u8* incoming_byte, void* user_data);
    static bool PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* user_data);

private:
    static u8 ComputeParity(u8 scan_code);
    static void BuildFrame(u8 scan_code, u8 frame[2]);

    u8 m_queue[k_queue_size];
    u8 m_queue_head;
    u8 m_queue_tail;
    u8 m_last_serial_data;
    u32 m_transfer_id;
};

#endif /* PS2_KEYBOARD_H */
