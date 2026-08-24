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

#ifndef LINK_CABLE_WIRE_H
#define LINK_CABLE_WIRE_H

#include <atomic>
#include "link_cable.h"

#define LINK_CABLE_STATE_COUNT 64
#define LINK_CABLE_TRANSFER_COUNT 64

#define LINK_CABLE_TRANSFER_TARGET_ARMED 0x01
#define LINK_CABLE_TRANSFER_CLOCK_CONFLICT 0x02
#define LINK_CABLE_TRANSFER_CGB_FAST 0x04

struct LinkCableLocalState
{
    u64 cycle;
    u8 sb;
    u8 sc;
};

struct LinkCableLocalTransfer
{
    u32 source_generation;
    u32 target_generation;
    u32 target_slot;
    u32 transfer_id;
    u64 request_cycle;
    u64 first_shift_cycle;
    u32 bit_cycles;
    u8 source_byte;
    u8 target_byte;
    u32 flags;
};

struct LinkCableSharedState
{
    std::atomic<u32> sequence;
    std::atomic<u32> generation;
    std::atomic<u64> cycle;
    std::atomic<u32> serial;

    LinkCableSharedState() : sequence(0), generation(0), cycle(0), serial(0) {}
};

struct LinkCableSharedTransfer
{
    std::atomic<u32> sequence;
    std::atomic<u32> source_generation;
    std::atomic<u32> target_generation;
    std::atomic<u32> target_slot;
    std::atomic<u32> transfer_id;
    std::atomic<u64> request_cycle;
    std::atomic<u64> first_shift_cycle;
    std::atomic<u32> bit_cycles;
    std::atomic<u32> data;
    std::atomic<u32> flags;

    LinkCableSharedTransfer() : sequence(0), source_generation(0),
        target_generation(0), target_slot(0), transfer_id(0),
        request_cycle(0), first_shift_cycle(0), bit_cycles(0), data(0),
        flags(0) {}
};

static_assert(alignof(LinkCableSharedState) >= alignof(std::atomic<u64>),
    "Link cable shared states require aligned 64-bit atomics");
static_assert(alignof(LinkCableSharedTransfer) >= alignof(std::atomic<u64>),
    "Link cable shared transfers require aligned 64-bit atomics");

inline u32 link_cable_pack_serial(u8 sb, u8 sc)
{
    return (u32)sb | ((u32)sc << 8);
}

inline u32 link_cable_pack_data(u8 source_byte, u8 target_byte)
{
    return (u32)source_byte | ((u32)target_byte << 8);
}

inline bool link_cable_shared_state_atomics_lock_free( const LinkCableSharedState& state)
{
    return state.sequence.is_lock_free() && state.generation.is_lock_free() &&
        state.cycle.is_lock_free() && state.serial.is_lock_free();
}

inline bool link_cable_shared_transfer_atomics_lock_free(const LinkCableSharedTransfer& transfer)
{
    return transfer.sequence.is_lock_free() &&
        transfer.source_generation.is_lock_free() &&
        transfer.target_generation.is_lock_free() &&
        transfer.target_slot.is_lock_free() &&
        transfer.transfer_id.is_lock_free() &&
        transfer.request_cycle.is_lock_free() &&
        transfer.first_shift_cycle.is_lock_free() &&
        transfer.bit_cycles.is_lock_free() && transfer.data.is_lock_free() &&
        transfer.flags.is_lock_free();
}

inline void link_cable_publish_shared_state(LinkCableSharedState& state, u32 generation, u64 cycle, u8 sb, u8 sc)
{
    u32 sequence = state.sequence.load(std::memory_order_relaxed);
    u32 busy_sequence = (sequence + 1) | 1u;

    state.sequence.exchange(busy_sequence, std::memory_order_acq_rel);
    state.generation.store(generation, std::memory_order_relaxed);
    state.cycle.store(cycle, std::memory_order_relaxed);
    state.serial.store(link_cable_pack_serial(sb, sc), std::memory_order_relaxed);
    state.sequence.store(busy_sequence + 1, std::memory_order_release);
}

inline bool link_cable_read_shared_state(const LinkCableSharedState& source, u32 expected_generation, LinkCableLocalState& state)
{
    u32 before = source.sequence.load(std::memory_order_acquire);

    if ((before & 1) != 0)
        return false;

    u32 generation = source.generation.load(std::memory_order_relaxed);
    state.cycle = source.cycle.load(std::memory_order_relaxed);
    u32 serial = source.serial.load(std::memory_order_relaxed);
    state.sb = (u8)(serial & 0xFF);
    state.sc = (u8)((serial >> 8) & 0xFF);

    std::atomic_thread_fence(std::memory_order_acquire);
    u32 after = source.sequence.load(std::memory_order_relaxed);

    return before == after && (after & 1) == 0 && generation == expected_generation;
}

inline void link_cable_publish_shared_transfer(
    LinkCableSharedTransfer& transfer, const LinkCableLocalTransfer& local)
{
    u32 sequence = transfer.sequence.load(std::memory_order_relaxed);
    u32 busy_sequence = (sequence + 1) | 1u;

    transfer.sequence.exchange(busy_sequence, std::memory_order_acq_rel);
    transfer.source_generation.store(local.source_generation, std::memory_order_relaxed);
    transfer.target_generation.store(local.target_generation, std::memory_order_relaxed);
    transfer.target_slot.store(local.target_slot, std::memory_order_relaxed);
    transfer.transfer_id.store(local.transfer_id, std::memory_order_relaxed);
    transfer.request_cycle.store(local.request_cycle, std::memory_order_relaxed);
    transfer.first_shift_cycle.store(local.first_shift_cycle, std::memory_order_relaxed);
    transfer.bit_cycles.store(local.bit_cycles, std::memory_order_relaxed);
    transfer.data.store(link_cable_pack_data(local.source_byte, local.target_byte), std::memory_order_relaxed);
    transfer.flags.store(local.flags, std::memory_order_relaxed);
    transfer.sequence.store(busy_sequence + 1, std::memory_order_release);
}

inline bool link_cable_read_shared_transfer(const LinkCableSharedTransfer& source,
    u32 expected_generation, LinkCableLocalTransfer& transfer)
{
    u32 before = source.sequence.load(std::memory_order_acquire);

    if ((before & 1) != 0)
        return false;

    transfer.source_generation = source.source_generation.load(std::memory_order_relaxed);
    transfer.target_generation = source.target_generation.load(std::memory_order_relaxed);
    transfer.target_slot = source.target_slot.load(std::memory_order_relaxed);
    transfer.transfer_id = source.transfer_id.load(std::memory_order_relaxed);
    transfer.request_cycle = source.request_cycle.load(std::memory_order_relaxed);
    transfer.first_shift_cycle = source.first_shift_cycle.load(std::memory_order_relaxed);
    transfer.bit_cycles = source.bit_cycles.load(std::memory_order_relaxed);
    u32 data = source.data.load(std::memory_order_relaxed);
    transfer.source_byte = (u8)(data & 0xFF);
    transfer.target_byte = (u8)((data >> 8) & 0xFF);
    transfer.flags = source.flags.load(std::memory_order_relaxed);

    std::atomic_thread_fence(std::memory_order_acquire);
    u32 after = source.sequence.load(std::memory_order_relaxed);

    return before == after && (after & 1) == 0 && transfer.source_generation == expected_generation;
}

#endif /* LINK_CABLE_WIRE_H */
