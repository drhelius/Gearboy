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

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "link_cable/link_cable_wire.h"

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

int main()
{
    LinkCableSharedState shared_state;
    LinkCableSharedTransfer shared_transfer;
    Check(link_cable_shared_state_atomics_lock_free(shared_state),
        "shared state atomics are lock-free");
    Check(link_cable_shared_transfer_atomics_lock_free(shared_transfer),
        "shared transfer atomics are lock-free");

    link_cable_publish_shared_state(shared_state, 7, 1234, 0xA5, 0xFE);
    LinkCableLocalState state;
    Check(link_cable_read_shared_state(shared_state, 7, state),
        "read a published state");
    Check(state.cycle == 1234 && state.sb == 0xA5 && state.sc == 0xFE,
        "state packing preserves cycle, SB, and SC");
    Check(!link_cable_read_shared_state(shared_state, 8, state),
        "state generation mismatch is rejected");

    shared_state.sequence.store(1, std::memory_order_relaxed);
    Check(!link_cable_read_shared_state(shared_state, 7, state),
        "odd state sequence is rejected");
    link_cable_publish_shared_state(shared_state, 9, 4321, 0x3C, 0xFF);
    Check(link_cable_read_shared_state(shared_state, 9, state),
        "state publication recovers an abandoned odd sequence");

    LinkCableLocalTransfer published = {};
    published.source_generation = 11;
    published.target_generation = 12;
    published.target_slot = 1;
    published.transfer_id = 99;
    published.request_cycle = 2000;
    published.first_shift_cycle = 2016;
    published.bit_cycles = 16;
    published.source_byte = 0x96;
    published.target_byte = 0x69;
    published.flags = LINK_CABLE_TRANSFER_TARGET_ARMED |
        LINK_CABLE_TRANSFER_CGB_FAST;
    link_cable_publish_shared_transfer(shared_transfer, published);

    LinkCableLocalTransfer recovered;
    Check(link_cable_read_shared_transfer(shared_transfer, 11, recovered),
        "read a published transfer");
    Check(recovered.target_generation == 12 && recovered.target_slot == 1 &&
        recovered.transfer_id == 99 && recovered.request_cycle == 2000 &&
        recovered.first_shift_cycle == 2016 && recovered.bit_cycles == 16 &&
        recovered.source_byte == 0x96 && recovered.target_byte == 0x69 &&
        recovered.flags == published.flags,
        "transfer packing preserves the complete descriptor");
    Check(!link_cable_read_shared_transfer(shared_transfer, 10, recovered),
        "transfer generation mismatch is rejected");

    std::atomic<bool> done(false);
    std::atomic<bool> failed(false);
    std::atomic<u32> reads(0);

    std::thread reader([&shared_transfer, &done, &failed, &reads]() {
        while (!done.load(std::memory_order_acquire))
        {
            LinkCableLocalTransfer current;
            if (!link_cable_read_shared_transfer(shared_transfer, 21, current))
                continue;

            u32 id = current.transfer_id;
            if (current.request_cycle != (u64)id * 100 ||
                current.first_shift_cycle != (u64)id * 100 + 8 ||
                current.bit_cycles != ((id & 1) ? 8u : 16u) ||
                current.source_byte != (u8)id ||
                current.target_byte != (u8)(id ^ 0xFF))
            {
                failed.store(true, std::memory_order_release);
            }

            reads.fetch_add(1, std::memory_order_relaxed);
        }
    });

    for (u32 i = 1; i <= 200000; i++)
    {
        LinkCableLocalTransfer current = {};
        current.source_generation = 21;
        current.target_generation = 22;
        current.target_slot = 1;
        current.transfer_id = i;
        current.request_cycle = (u64)i * 100;
        current.first_shift_cycle = (u64)i * 100 + 8;
        current.bit_cycles = (i & 1) ? 8 : 16;
        current.source_byte = (u8)i;
        current.target_byte = (u8)(i ^ 0xFF);
        current.flags = LINK_CABLE_TRANSFER_TARGET_ARMED;
        link_cable_publish_shared_transfer(shared_transfer, current);

        if ((i & 0xFF) == 0)
            std::this_thread::yield();
    }

    done.store(true, std::memory_order_release);
    reader.join();

    Check(reads.load(std::memory_order_acquire) > 0,
        "concurrent transfer reader accepts snapshots");
    Check(!failed.load(std::memory_order_acquire),
        "concurrent transfer snapshots remain consistent");

    printf("Gearboy link cable wire tests passed\n");
    return 0;
}
