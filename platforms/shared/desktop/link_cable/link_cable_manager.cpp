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
#include <chrono>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "link_cable_manager.h"
#include "link_cable_wire.h"
#include "log.h"

#define LINK_CABLE_SHM_MAGIC 0x47424C4B
#define LINK_CABLE_SHM_VERSION 1
#define LINK_CABLE_BARRIER_SLEEP_US 100
#if !defined(LINK_CABLE_SHM_PREFIX)
#define LINK_CABLE_SHM_PREFIX "gearboy-link-"
#endif

static u64 link_cable_saturating_add(u64 value, u64 add)
{
    return add > (~0ULL - value) ? ~0ULL : value + add;
}

struct LinkCableManager::Shared
{
    struct Peer
    {
        std::atomic<u32> state;
        std::atomic<u32> generation;
        std::atomic<u64> heartbeat_us;
        std::atomic<u64> committed_cycle;
        std::atomic<u64> promise_cycle;
        std::atomic<u32> state_write_index;
        std::atomic<u32> transfer_write_index;
        LinkCableSharedState states[LINK_CABLE_STATE_COUNT];
        LinkCableSharedTransfer transfers[LINK_CABLE_TRANSFER_COUNT];

        Peer() : state(0), generation(0), heartbeat_us(0),
            committed_cycle(0), promise_cycle(0), state_write_index(0),
            transfer_write_index(0) {}
    };

    std::atomic<u32> magic;
    u32 version;
    u8 session;
    u8 reserved[3];
    Peer peers[LINK_CABLE_MAX_PEERS];

    Shared() : magic(0), version(LINK_CABLE_SHM_VERSION), session(0)
    {
        memset(reserved, 0, sizeof(reserved));
    }
};

LinkCableManager::LinkCableManager()
{
    m_shared = NULL;
    m_mapping_handle = NULL;
    m_mapping_fd = -1;
    m_slot = -1;
    m_generation = 0;
    m_session = 0;
    m_local_anchor = 0;
    m_bus_anchor = 0;
    m_last_sync_exit_us = 0;
    m_remote_slot = -1;
    m_remote_generation = 0;
    m_transfer_read_index = 0;
    m_normal_barrier_stall_us = link_cable_normal_barrier_stall_us();
    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = LinkCableModeDisabled;
}

LinkCableManager::~LinkCableManager()
{
    Stop();
}

bool LinkCableManager::Connect(u8 session, u64 local_cycle)
{
    Stop();

    if (session == 0)
    {
        SetFault("Link cable session must be between 1 and 255");
        return false;
    }

    if (!Map(session))
        return false;

    m_session = session;

    if (!ClaimSlot(local_cycle, false))
    {
        Unmap();
        SetFault("Link cable session is full");
        return false;
    }

    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = LinkCableModeConnected;
    m_status.session = session;
    m_status.attachments = 1;

    RefreshStatus();

    Log("Link cable: connected to shared session %u as peer %u", session, m_slot + 1);

    return true;
}

void LinkCableManager::Stop()
{
    if (m_shared && m_slot >= 0)
    {
        Shared::Peer& peer = m_shared->peers[m_slot];

        if (peer.generation.load(std::memory_order_acquire) == m_generation)
            peer.state.store(0, std::memory_order_release);
    }

    Unmap();

    m_slot = -1;
    m_generation = 0;
    m_session = 0;
    m_remote_slot = -1;
    m_remote_generation = 0;
    m_transfer_read_index = 0;

    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = LinkCableModeDisabled;
}

void LinkCableManager::PublishPortState(u64 local_cycle, u8 sb, u8 sc)
{
    if (!EnsureAttached(local_cycle))
        return;

    Shared::Peer& peer = m_shared->peers[m_slot];
    peer.heartbeat_us.store(GetClockMicroseconds(), std::memory_order_release);

    u32 index = peer.state_write_index.load(std::memory_order_relaxed);
    LinkCableSharedState& state = peer.states[index % LINK_CABLE_STATE_COUNT];

    link_cable_publish_shared_state(state, m_generation, ToBusCycle(local_cycle), sb, sc);
    peer.state_write_index.store(index + 1, std::memory_order_release);
    m_status.states_published++;
}

void LinkCableManager::StartTransfer(u64 request_cycle, u64 first_shift_cycle,
    u32 bit_cycles, u8 outgoing_byte, u32 transfer_id, u8* incoming_byte)
{
    if (!incoming_byte)
        return;

    *incoming_byte = 0xFF;

    if (!EnsureAttached(request_cycle) || bit_cycles == 0)
        return;

    u64 request_bus_cycle = ToBusCycle(request_cycle);
    u64 first_shift_bus_cycle = ToBusCycle(first_shift_cycle);
    u64 promise_distance = first_shift_bus_cycle > request_bus_cycle ? first_shift_bus_cycle - request_bus_cycle : 0;
    CommitLocal(request_bus_cycle, (u32)MIN((u64)LINK_CABLE_MAX_PROMISE_CYCLES, promise_distance));

    int peer_slot = -1;
    u32 peer_generation = 0;
    ReapStalePeers(GetClockMicroseconds());

    if (!FindPeer(&peer_slot, &peer_generation))
    {
        m_status.transfers_no_peer++;
        return;
    }

    if (!WaitForPeerCommit(peer_slot, peer_generation, request_bus_cycle))
    {
        m_status.transfers_no_peer++;
        return;
    }

    u8 peer_sb = 0xFF;
    u8 peer_sc = 0x00;
    bool state_found = ReadPeerStateAt(peer_slot, peer_generation, request_bus_cycle, &peer_sb, &peer_sc);
    bool target_armed = state_found && ((peer_sc & 0x81) == 0x80);
    bool clock_conflict = state_found && ((peer_sc & 0x81) == 0x81);

    if (target_armed)
        *incoming_byte = peer_sb;
    else
        m_status.transfers_unarmed++;

    if (clock_conflict)
        m_status.clock_conflicts++;

    LinkCableLocalTransfer transfer = {};
    transfer.source_generation = m_generation;
    transfer.target_generation = peer_generation;
    transfer.target_slot = (u32)peer_slot;
    transfer.transfer_id = transfer_id;
    transfer.request_cycle = request_bus_cycle;
    transfer.first_shift_cycle = first_shift_bus_cycle;
    transfer.bit_cycles = bit_cycles;
    transfer.source_byte = outgoing_byte;
    transfer.target_byte = target_armed ? peer_sb : 0xFF;
    transfer.flags = target_armed ? LINK_CABLE_TRANSFER_TARGET_ARMED : 0;

    if (clock_conflict)
        transfer.flags |= LINK_CABLE_TRANSFER_CLOCK_CONFLICT;
    if (bit_cycles <= 16)
        transfer.flags |= LINK_CABLE_TRANSFER_CGB_FAST;

    PublishTransfer(transfer);
}

bool LinkCableManager::PollTransfer(u64 local_cycle, GB_LinkCableTransfer* transfer)
{
    if (!transfer || !EnsureAttached(local_cycle))
        return false;

    u64 now = GetClockMicroseconds();
    m_shared->peers[m_slot].heartbeat_us.store(now, std::memory_order_release);
    ReapStalePeers(now);

    int peer_slot = -1;
    u32 peer_generation = 0;

    if (!FindPeer(&peer_slot, &peer_generation))
    {
        m_remote_slot = -1;
        m_remote_generation = 0;
        m_transfer_read_index = 0;
        return false;
    }

    Shared::Peer& peer = m_shared->peers[peer_slot];

    if (peer_slot != m_remote_slot || peer_generation != m_remote_generation)
    {
        m_remote_slot = peer_slot;
        m_remote_generation = peer_generation;
        m_transfer_read_index = peer.transfer_write_index.load(std::memory_order_acquire);
        return false;
    }

    u32 write_index = peer.transfer_write_index.load(std::memory_order_acquire);

    if (write_index - m_transfer_read_index > LINK_CABLE_TRANSFER_COUNT)
    {
        m_status.transfer_ring_overruns++;
        m_transfer_read_index = write_index - LINK_CABLE_TRANSFER_COUNT;
    }

    while (m_transfer_read_index != write_index)
    {
        LinkCableSharedTransfer& source = peer.transfers[m_transfer_read_index % LINK_CABLE_TRANSFER_COUNT];
        LinkCableLocalTransfer local;

        if (!link_cable_read_shared_transfer(source, peer_generation, local))
        {
            m_status.seqlock_retries++;
            return false;
        }

        m_transfer_read_index++;

        if (local.target_slot != (u32)m_slot || local.target_generation != m_generation)
            continue;

        if ((local.flags & LINK_CABLE_TRANSFER_TARGET_ARMED) == 0)
            continue;

        transfer->request_cycle = FromBusCycle(local.request_cycle);
        transfer->first_shift_cycle = FromBusCycle(local.first_shift_cycle);
        transfer->bit_cycles = local.bit_cycles;
        transfer->transfer_id = local.transfer_id;
        transfer->incoming_byte = local.source_byte;
        transfer->local_byte = local.target_byte;

        if (local_cycle > transfer->first_shift_cycle)
            m_status.late_descriptors++;

        m_status.transfers_delivered++;
        return true;
    }

    return false;
}

void LinkCableManager::Synchronize(u64 local_cycle, u32 promise_cycles)
{
    if (!EnsureAttached(local_cycle))
        return;

    m_status.sync_calls++;
    u64 now = GetClockMicroseconds();

    if (m_last_sync_exit_us != 0)
    {
        u64 gap = now - m_last_sync_exit_us;
        m_status.sync_gap_max_us = MAX(m_status.sync_gap_max_us, gap);
        if (gap >= 50000)
            m_status.sync_gap_over_50ms++;
    }

    ReapStalePeers(now);

    Shared::Peer& local = m_shared->peers[m_slot];
    u64 cycle = ToBusCycle(local_cycle);
    CommitLocal(cycle, MIN(promise_cycles, (u32)LINK_CABLE_MAX_PROMISE_CYCLES));

    u64 wait_started = 0;
    u64 progress_time = now;
    u64 previous_floor = 0;

    for (;;)
    {
        u64 floor = ~0ULL;

        for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
        {
            Shared::Peer& peer = m_shared->peers[i];

            if (peer.state.load(std::memory_order_acquire) == 1)
            {
                floor = MIN(floor,
                    peer.promise_cycle.load(std::memory_order_acquire));
            }
        }

        if (floor == ~0ULL || cycle <= floor)
            break;

        if (wait_started == 0)
        {
            wait_started = GetClockMicroseconds();
            m_status.barrier_waits++;
        }

        now = GetClockMicroseconds();

        if (floor != previous_floor)
        {
            previous_floor = floor;
            progress_time = now;
        }

        ReapStalePeers(now);
        local.heartbeat_us.store(now, std::memory_order_release);

        if (now - progress_time >= m_normal_barrier_stall_us)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(LINK_CABLE_BARRIER_SLEEP_US));
            m_status.sleep_calls++;
        }
        else
        {
            std::this_thread::yield();
            m_status.spin_iterations++;
        }
    }

    now = GetClockMicroseconds();

    if (wait_started != 0)
        RecordBarrierWait(now - wait_started);

    m_last_sync_exit_us = now;
}

bool LinkCableManager::IsActive() const
{
    return m_shared != NULL && m_slot >= 0;
}

bool LinkCableManager::IsCableConnected() const
{
    return IsActive();
}

bool LinkCableManager::IsPacingPeer() const
{
    if (!IsActive())
        return false;

    for (int i = 0; i < m_slot; i++)
    {
        if (m_shared->peers[i].state.load(std::memory_order_acquire) == 1)
            return false;
    }

    return true;
}

LinkCableStatus LinkCableManager::GetStatus()
{
    RefreshStatus();
    return m_status;
}

void LinkCableManager::ResetMetrics()
{
    m_status.states_published = 0;
    m_status.transfers_published = 0;
    m_status.transfers_delivered = 0;
    m_status.transfers_no_peer = 0;
    m_status.transfers_unarmed = 0;
    m_status.clock_conflicts = 0;
    m_status.late_descriptors = 0;
    m_status.sync_calls = 0;
    m_status.snapshot_waits = 0;
    m_status.snapshot_wait_us = 0;
    m_status.barrier_waits = 0;
    m_status.barrier_wait_us = 0;
    m_status.barrier_wait_max_us = 0;
    m_status.barrier_wait_over_1ms = 0;
    m_status.barrier_wait_over_10ms = 0;
    m_status.barrier_wait_over_50ms = 0;
    m_status.spin_iterations = 0;
    m_status.sleep_calls = 0;
    m_status.sync_gap_max_us = 0;
    m_status.sync_gap_over_50ms = 0;
    m_status.peer_detaches = 0;
    m_status.peer_detach_max_age_us = 0;
    m_status.slot_reclaims = 0;
    m_status.seqlock_retries = 0;
    m_status.state_ring_overruns = 0;
    m_status.transfer_ring_overruns = 0;
    m_status.attachments = 0;
    m_last_sync_exit_us = GetClockMicroseconds();
}

void LinkCableManager::SetNormalBarrierStallUs(u32 stall_us)
{
    m_normal_barrier_stall_us = stall_us;
}

bool LinkCableManager::Map(u8 session)
{
    char name[64];
    bool created;

#if defined(_WIN32)
    snprintf(name, sizeof(name), "Local\\" LINK_CABLE_SHM_PREFIX "%u", session);

    HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, (DWORD)sizeof(Shared), name);

    if (!mapping)
    {
        SetFault("Failed to create link cable shared memory");
        return false;
    }

    created = GetLastError() != ERROR_ALREADY_EXISTS;
    void* address = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared));

    if (!address)
    {
        CloseHandle(mapping);
        SetFault("Failed to map link cable shared memory");
        return false;
    }

    m_mapping_handle = mapping;
    m_shared = (Shared*)address;
#else
    snprintf(name, sizeof(name), "/" LINK_CABLE_SHM_PREFIX "%u", session);

    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    created = fd >= 0;

    if (!created && errno == EEXIST)
        fd = shm_open(name, O_RDWR, 0600);

    if (fd < 0)
    {
        SetFault("Failed to open link cable shared memory");
        return false;
    }

    if (created && ftruncate(fd, sizeof(Shared)) != 0)
    {
        close(fd);
        shm_unlink(name);
        SetFault("Failed to size link cable shared memory");
        return false;
    }

    if (!created)
    {
        u64 started = GetClockMicroseconds();
        struct stat status;

        while (fstat(fd, &status) != 0 || status.st_size < (off_t)sizeof(Shared))
        {
            if (GetClockMicroseconds() - started > LINK_CABLE_DETACH_US)
            {
                close(fd);
                SetFault("Link cable shared memory sizing timed out");
                return false;
            }

            std::this_thread::yield();
        }
    }

    void* address = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (address == MAP_FAILED)
    {
        close(fd);
        SetFault("Failed to map link cable shared memory");
        return false;
    }

    m_mapping_fd = fd;
    m_shared = (Shared*)address;
#endif

    if (created)
    {
        new (m_shared) Shared();
        m_shared->session = session;
        m_shared->magic.store(LINK_CABLE_SHM_MAGIC, std::memory_order_release);
    }
    else
    {
        u64 started = GetClockMicroseconds();

        while (m_shared->magic.load(std::memory_order_acquire) != LINK_CABLE_SHM_MAGIC)
        {
            if (GetClockMicroseconds() - started > LINK_CABLE_DETACH_US)
            {
                Unmap();
                SetFault("Link cable shared memory initialization timed out");
                return false;
            }

            std::this_thread::yield();
        }

        if (m_shared->version != LINK_CABLE_SHM_VERSION || m_shared->session != session)
        {
            Unmap();
            SetFault("Incompatible link cable shared memory");
            return false;
        }
    }

    if (!SharedAtomicsLockFree())
    {
        Unmap();
        SetFault("Link cable shared atomics are not lock-free");
        return false;
    }

    return true;
}

void LinkCableManager::Unmap()
{
    if (!m_shared)
        return;

#if defined(_WIN32)
    UnmapViewOfFile(m_shared);

    if (m_mapping_handle)
        CloseHandle((HANDLE)m_mapping_handle);
#else
    munmap(m_shared, sizeof(Shared));

    if (m_mapping_fd >= 0)
        close(m_mapping_fd);
#endif

    m_shared = NULL;
    m_mapping_handle = NULL;
    m_mapping_fd = -1;
}

bool LinkCableManager::ClaimSlot(u64 local_cycle, bool reattach)
{
    u64 now = GetClockMicroseconds();

    ReapStalePeers(now, true);

    u64 bus_anchor = 0;

    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        Shared::Peer& peer = m_shared->peers[i];

        if (peer.state.load(std::memory_order_acquire) == 1)
        {
            bus_anchor = MAX(bus_anchor, peer.committed_cycle.load(std::memory_order_acquire));
            bus_anchor = MAX(bus_anchor, peer.promise_cycle.load(std::memory_order_acquire));
        }
    }

    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        Shared::Peer& peer = m_shared->peers[i];

        u32 expected = 0;
        if (!peer.state.compare_exchange_strong(expected, 2, std::memory_order_acq_rel))
            continue;

        m_slot = i;
        m_generation = peer.generation.fetch_add(1, std::memory_order_acq_rel) + 1;
        m_local_anchor = local_cycle;
        m_bus_anchor = bus_anchor;
        m_remote_slot = -1;
        m_remote_generation = 0;
        m_transfer_read_index = 0;

        peer.state_write_index.store(0, std::memory_order_relaxed);
        peer.transfer_write_index.store(0, std::memory_order_relaxed);
        peer.committed_cycle.store(bus_anchor, std::memory_order_relaxed);
        peer.promise_cycle.store(link_cable_saturating_add(bus_anchor, LINK_CABLE_MAX_PROMISE_CYCLES), std::memory_order_relaxed);
        peer.heartbeat_us.store(now, std::memory_order_relaxed);
        peer.state.store(1, std::memory_order_release);

        if (reattach)
            m_status.slot_reclaims++;

        m_status.attachments++;

        return true;
    }

    return false;
}

bool LinkCableManager::EnsureAttached(u64 local_cycle)
{
    if (!m_shared || m_status.mode == LinkCableModeFault)
        return false;

    if (m_slot >= 0)
    {
        Shared::Peer& peer = m_shared->peers[m_slot];

        if (peer.state.load(std::memory_order_acquire) == 1 && peer.generation.load(std::memory_order_acquire) == m_generation)
        {
            return true;
        }
    }

    m_slot = -1;

    if (ClaimSlot(local_cycle, true))
        return true;

    SetFault("Lost link cable slot and could not reclaim it");
    return false;
}

void LinkCableManager::ReapStalePeers(u64 now_us, bool preserve_idle)
{
    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        if (i == m_slot)
            continue;

        Shared::Peer& peer = m_shared->peers[i];

        u64 heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);
        u32 generation = peer.generation.load(std::memory_order_acquire);
        u64 age = link_cable_heartbeat_age(now_us, heartbeat);

        if (peer.state.load(std::memory_order_acquire) != 1 || age <= LINK_CABLE_DETACH_US)
            continue;

        if (preserve_idle && peer.transfer_write_index.load(std::memory_order_acquire) == 0)
            continue;

        if (peer.state.load(std::memory_order_acquire) != 1)
            continue;

        u32 current_generation = peer.generation.load(
            std::memory_order_acquire);

        u64 current_heartbeat = peer.heartbeat_us.load(
            std::memory_order_acquire);

        if (!link_cable_lease_is_unchanged_and_stale(now_us, heartbeat, generation, current_heartbeat, current_generation))
            continue;

        u32 expected = 1;

        if (peer.state.compare_exchange_strong(expected, 0,
            std::memory_order_acq_rel))
        {
            m_status.peer_detaches++;
            m_status.peer_detach_max_age_us = MAX(m_status.peer_detach_max_age_us, age);
        }
    }
}

bool LinkCableManager::FindPeer(int* slot, u32* generation)
{
    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        if (i == m_slot)
            continue;

        Shared::Peer& peer = m_shared->peers[i];

        if (peer.state.load(std::memory_order_acquire) == 1)
        {
            *slot = i;
            *generation = peer.generation.load(std::memory_order_acquire);
            return true;
        }
    }

    return false;
}

bool LinkCableManager::ReadPeerStateAt(int slot, u32 generation,
    u64 bus_cycle, u8* sb, u8* sc)
{
    Shared::Peer& peer = m_shared->peers[slot];
    u32 write_index = peer.state_write_index.load(std::memory_order_acquire);
    u32 count = MIN(write_index, (u32)LINK_CABLE_STATE_COUNT);

    for (u32 offset = 0; offset < count; offset++)
    {
        LinkCableSharedState& source = peer.states[(write_index - 1 - offset) % LINK_CABLE_STATE_COUNT];
        LinkCableLocalState state;

        if (!link_cable_read_shared_state(source, generation, state))
        {
            m_status.seqlock_retries++;
            continue;
        }

        if (state.cycle <= bus_cycle)
        {
            *sb = state.sb;
            *sc = state.sc;
            return true;
        }
    }

    if (write_index > LINK_CABLE_STATE_COUNT)
        m_status.state_ring_overruns++;

    return false;
}

bool LinkCableManager::WaitForPeerCommit(int slot, u32 generation, u64 bus_cycle)
{
    Shared::Peer& peer = m_shared->peers[slot];
    u64 started = 0;
    u64 progress_time = GetClockMicroseconds();
    u64 previous_commit = 0;

    for (;;)
    {
        if (peer.state.load(std::memory_order_acquire) != 1 ||
            peer.generation.load(std::memory_order_acquire) != generation)
        {
            break;
        }

        u64 committed = peer.committed_cycle.load(std::memory_order_acquire);
        if (committed >= bus_cycle)
        {
            if (started != 0)
                m_status.snapshot_wait_us += GetClockMicroseconds() - started;
            return true;
        }

        u64 now = GetClockMicroseconds();
        if (started == 0)
        {
            started = now;
            m_status.snapshot_waits++;
        }

        if (committed != previous_commit)
        {
            previous_commit = committed;
            progress_time = now;
        }

        ReapStalePeers(now);

        if (m_slot >= 0)
        {
            m_shared->peers[m_slot].heartbeat_us.store(now,std::memory_order_release);
        }

        if (now - progress_time >= m_normal_barrier_stall_us)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(LINK_CABLE_BARRIER_SLEEP_US));
            m_status.sleep_calls++;
        }
        else
        {
            std::this_thread::yield();
            m_status.spin_iterations++;
        }
    }

    if (started != 0)
        m_status.snapshot_wait_us += GetClockMicroseconds() - started;

    return false;
}

void LinkCableManager::CommitLocal(u64 bus_cycle, u32 promise_cycles)
{
    Shared::Peer& local = m_shared->peers[m_slot];
    u64 now = GetClockMicroseconds();

    local.heartbeat_us.store(now, std::memory_order_release);
    local.committed_cycle.store(bus_cycle, std::memory_order_release);
    local.promise_cycle.store(link_cable_saturating_add(bus_cycle, promise_cycles), std::memory_order_release);
}

void LinkCableManager::PublishTransfer(const LinkCableLocalTransfer& transfer)
{
    Shared::Peer& peer = m_shared->peers[m_slot];
    u32 index = peer.transfer_write_index.load(std::memory_order_relaxed);
    LinkCableSharedTransfer& shared_transfer = peer.transfers[index % LINK_CABLE_TRANSFER_COUNT];

    link_cable_publish_shared_transfer(shared_transfer, transfer);
    peer.transfer_write_index.store(index + 1, std::memory_order_release);
    m_status.transfers_published++;
}

bool LinkCableManager::SharedAtomicsLockFree() const
{
    if (!m_shared->magic.is_lock_free())
        return false;

    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        const Shared::Peer& peer = m_shared->peers[i];
        if (!peer.state.is_lock_free() || !peer.generation.is_lock_free() ||
            !peer.heartbeat_us.is_lock_free() ||
            !peer.committed_cycle.is_lock_free() ||
            !peer.promise_cycle.is_lock_free() ||
            !peer.state_write_index.is_lock_free() ||
            !peer.transfer_write_index.is_lock_free())
        {
            return false;
        }

        for (int state = 0; state < LINK_CABLE_STATE_COUNT; state++)
        {
            if (!link_cable_shared_state_atomics_lock_free(
                peer.states[state]))
            {
                return false;
            }
        }

        for (int transfer = 0; transfer < LINK_CABLE_TRANSFER_COUNT; transfer++)
        {
            if (!link_cable_shared_transfer_atomics_lock_free(peer.transfers[transfer]))
            {
                return false;
            }
        }
    }

    return true;
}

u64 LinkCableManager::ToBusCycle(u64 local_cycle) const
{
    if (local_cycle >= m_local_anchor)
    {
        return link_cable_saturating_add(m_bus_anchor, local_cycle - m_local_anchor);
    }

    u64 distance = m_local_anchor - local_cycle;
    return distance > m_bus_anchor ? 0 : m_bus_anchor - distance;
}

u64 LinkCableManager::FromBusCycle(u64 bus_cycle) const
{
    if (bus_cycle >= m_bus_anchor)
    {
        return link_cable_saturating_add(m_local_anchor, bus_cycle - m_bus_anchor);
    }

    u64 distance = m_bus_anchor - bus_cycle;
    return distance > m_local_anchor ? 0 : m_local_anchor - distance;
}

u64 LinkCableManager::GetClockMicroseconds() const
{
    return (u64)std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void LinkCableManager::RecordBarrierWait(u64 wait)
{
    m_status.barrier_wait_us += wait;
    m_status.barrier_wait_max_us = MAX(m_status.barrier_wait_max_us, wait);

    if (wait >= 1000)
        m_status.barrier_wait_over_1ms++;
    if (wait >= 10000)
        m_status.barrier_wait_over_10ms++;
    if (wait >= 50000)
        m_status.barrier_wait_over_50ms++;
}

void LinkCableManager::SetFault(const char* message)
{
    m_status.mode = LinkCableModeFault;
    snprintf(m_status.last_error, sizeof(m_status.last_error), "%s", message);
    Error("Link cable: %s", message);
}

void LinkCableManager::RefreshStatus()
{
    if (!m_shared)
        return;

    int peers = 0;
    u8 local_peer_id = 0;

    for (int i = 0; i < LINK_CABLE_MAX_PEERS; i++)
    {
        Shared::Peer& peer = m_shared->peers[i];

        if (peer.state.load(std::memory_order_acquire) == 1)
        {
            peers++;

            if (i == m_slot)
                local_peer_id = (u8)peers;
        }
    }

    m_status.local_peer_id = local_peer_id;
    m_status.peer_count = peers;
    m_status.pacing_peer = IsPacingPeer();
}
