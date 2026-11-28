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

#include "PS2Keyboard.h"
#include <cstring>

PS2Keyboard::PS2Keyboard()
{
    Reset();
}

void PS2Keyboard::Reset()
{
    m_queue_head = 0;
    m_queue_tail = 0;
    m_last_serial_data = 0xFF;
    m_transfer_id = 0;
    memset(m_queue, 0, sizeof(m_queue));
}

u8 PS2Keyboard::ComputeParity(u8 scan_code)
{
    u8 ones = 0;

    for (int bit = 0; bit < 8; bit++)
    {
        if ((scan_code >> bit) & 0x01)
            ones++;
    }

    return (ones & 0x01) ? 0 : 1;
}

void PS2Keyboard::BuildFrame(u8 scan_code, u8 frame[2])
{
    const u8 parity = ComputeParity(scan_code);

    frame[0] = 0; // start bit
    for (int bit = 0; bit < 7; bit++)
        frame[0] |= ((scan_code >> bit) & 1) << (6-bit);

    frame[1] = (scan_code & 0x80) | (parity << 6) | (1 << 5);
}

bool PS2Keyboard::QueueScanCode(u8 scan_code)
{
    // make sure there's space in the queue for both bytes before writing one
    const int used = (m_queue_tail + k_queue_size - m_queue_head) % k_queue_size;
    if (k_queue_size - 1 - used < 2)
        return false;

    u8 frame[2];
    BuildFrame(scan_code, frame);

    for (int i = 0; i < 2; i++)
    {
        const u8 next_tail = (m_queue_tail + 1) % k_queue_size;
        if (next_tail == m_queue_head)
            return false;

        m_queue[m_queue_tail] = frame[i];
        m_queue_tail = next_tail;
    }

    return true;
}

void PS2Keyboard::StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent event, void* user_data)
{
    UNUSED(cycle);
    UNUSED(sc);
    UNUSED(event);

    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (keyboard)
        keyboard->m_last_serial_data = sb;
}

void PS2Keyboard::StartCallback(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles, u8 outgoing_byte,
    u32 transfer_id, u8* incoming_byte, void* user_data)
{
    UNUSED(request_cycle);
    UNUSED(first_shift_cycle);
    UNUSED(bit_cycles);
    UNUSED(outgoing_byte);
    UNUSED(transfer_id);

    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (!keyboard || !incoming_byte) return;

    const u8 head = keyboard->m_queue_head;
    if (head == keyboard->m_queue_tail) return;

    const u8 next_head = (head + 1) % k_queue_size;
    *incoming_byte = keyboard->m_queue[head];
    keyboard->m_queue_head = next_head;
    keyboard->m_last_serial_data = *incoming_byte;
}

bool PS2Keyboard::PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* user_data)
{
    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (!keyboard || !transfer)
        return false;

    const u8 head = keyboard->m_queue_head;
    if (head == keyboard->m_queue_tail)
        return false;

    const u8 next_head = (head + 1) % k_queue_size;
    transfer->incoming_byte = keyboard->m_queue[head];
    transfer->local_byte = keyboard->m_last_serial_data;
    transfer->request_cycle = cycle;
    transfer->first_shift_cycle = cycle + k_bit_cycles;
    transfer->bit_cycles = k_bit_cycles;
    transfer->transfer_id = ++keyboard->m_transfer_id;

    keyboard->m_queue_head = next_head;
    return true;
}
