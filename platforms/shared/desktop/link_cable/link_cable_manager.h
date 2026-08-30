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

#ifndef LINK_CABLE_MANAGER_H
#define LINK_CABLE_MANAGER_H

#include "link_cable.h"

#define LINK_CABLE_DETACH_US 500000

struct LinkCableLocalTransfer;

enum LinkCableMode
{
    LinkCableModeDisabled,
    LinkCableModeConnected,
    LinkCableModeFault
};

struct LinkCableStatus
{
    LinkCableMode mode;
    u8 session;
    u8 local_peer_id;
    int peer_count;
    u64 states_published;
    u64 transfers_published;
    u64 transfers_delivered;
    u64 transfers_no_peer;
    u64 transfers_unarmed;
    u64 clock_conflicts;
    u64 late_descriptors;
    u64 sync_calls;
    u64 snapshot_waits;
    u64 snapshot_wait_us;
    u64 barrier_waits;
    u64 barrier_wait_us;
    u64 barrier_wait_max_us;
    u64 barrier_wait_over_1ms;
    u64 barrier_wait_over_10ms;
    u64 barrier_wait_over_50ms;
    u64 spin_iterations;
    u64 sleep_calls;
    u64 sync_gap_max_us;
    u64 sync_gap_over_50ms;
    u64 peer_detaches;
    u64 peer_detach_max_age_us;
    u64 slot_reclaims;
    u64 seqlock_retries;
    u64 state_ring_overruns;
    u64 transfer_ring_overruns;
    u64 attachments;
    bool pacing_peer;
    char last_error[128];
};

class LinkCableManager
{
public:
    LinkCableManager();
    ~LinkCableManager();
    bool Connect(u8 session, u64 local_cycle);
    void Stop();
    void PublishPortState(u64 local_cycle, u8 sb, u8 sc);
    void StartTransfer(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles,
        u8 outgoing_byte, u32 transfer_id, u8* incoming_byte);
    bool PollTransfer(u64 local_cycle, GB_LinkCableTransfer* transfer);
    void Synchronize(u64 local_cycle, u32 promise_cycles);
    bool IsActive() const;
    bool IsCableConnected() const;
    bool IsPacingPeer() const;
    LinkCableStatus GetStatus();
    void ResetMetrics();
    void SetNormalBarrierStallUs(u32 stall_us);

private:
    struct Shared;

    bool Map(u8 session);
    void Unmap();
    bool ClaimSlot(u64 local_cycle, bool reattach);
    bool EnsureAttached(u64 local_cycle);
    void ReapStalePeers(u64 now_us, bool preserve_idle = false);
    bool FindPeer(int* slot, u32* generation);
    bool ReadPeerStateAt(int slot, u32 generation, u64 bus_cycle, u8* sb, u8* sc);
    bool WaitForPeerCommit(int slot, u32 generation, u64 bus_cycle);
    void CommitLocal(u64 bus_cycle, u32 promise_cycles);
    void PublishTransfer(const LinkCableLocalTransfer& transfer);
    bool SharedAtomicsLockFree() const;
    u64 ToBusCycle(u64 local_cycle) const;
    u64 FromBusCycle(u64 bus_cycle) const;
    u64 GetClockMicroseconds() const;
    void RecordBarrierWait(u64 wait);
    void SetFault(const char* message);
    void RefreshStatus();

private:
    Shared* m_shared;
    void* m_mapping_handle;
    int m_mapping_fd;
    int m_slot;
    u32 m_generation;
    u8 m_session;
    u64 m_local_anchor;
    u64 m_bus_anchor;
    u64 m_last_sync_exit_us;
    int m_remote_slot;
    u32 m_remote_generation;
    u32 m_transfer_read_index;
    LinkCableStatus m_status;
    u32 m_normal_barrier_stall_us;
};

inline u32 link_cable_normal_barrier_stall_us()
{
#if defined(_WIN32)
    return 5000;
#elif defined(__APPLE__)
    return 100;
#else
    return 250;
#endif
}

inline u64 link_cable_heartbeat_age(u64 now, u64 heartbeat)
{
    return heartbeat <= now ? now - heartbeat : 0;
}

inline bool link_cable_lease_is_unchanged_and_stale(u64 now, u64 observed_heartbeat,
    u32 observed_generation, u64 current_heartbeat, u32 current_generation)
{
    return current_heartbeat == observed_heartbeat &&
        current_generation == observed_generation &&
        link_cable_heartbeat_age(now, current_heartbeat) > LINK_CABLE_DETACH_US;
}

#endif /* LINK_CABLE_MANAGER_H */
