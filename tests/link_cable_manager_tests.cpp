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

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <thread>
#if !defined(_WIN32)
#include <sys/wait.h>
#include <unistd.h>
#endif
#include "link_cable/link_cable_manager.h"

bool g_mcp_stdio_mode = true;

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static u8 TestSession(int offset)
{
#if defined(_WIN32)
    return (u8)(180 + offset);
#else
    return (u8)(20 + ((getpid() + offset * 37) % 200));
#endif
}

static void TestHelpers()
{
    Check(link_cable_heartbeat_age(100, 101) == 0,
        "newer concurrent heartbeat has zero age");
    Check(link_cable_heartbeat_age(151, 100) == 51,
        "older heartbeat reports elapsed age");
    Check(link_cable_lease_is_unchanged_and_stale(
        LINK_CABLE_DETACH_US + 1, 0, 3, 0, 3),
        "unchanged expired lease remains stale");
    Check(!link_cable_lease_is_unchanged_and_stale(
        LINK_CABLE_DETACH_US, 0, 3, 0, 3),
        "lease remains valid through its deadline");
    Check(!link_cable_lease_is_unchanged_and_stale(
        LINK_CABLE_DETACH_US + 1, 0, 3, 1, 3),
        "refreshed heartbeat cancels eviction");
    Check(!link_cable_lease_is_unchanged_and_stale(
        LINK_CABLE_DETACH_US + 1, 0, 3, 0, 4),
        "changed generation cancels eviction");

#if defined(_WIN32)
    Check(link_cable_normal_barrier_stall_us() == 5000,
        "Windows barrier uses the expected stall threshold");
#elif defined(__APPLE__)
    Check(link_cable_normal_barrier_stall_us() == 100,
        "macOS barrier uses the expected stall threshold");
#else
    Check(link_cable_normal_barrier_stall_us() == 250,
        "other platforms use the expected stall threshold");
#endif
}

static void TestAttachAndTransfer()
{
    u8 session = TestSession(0);
    LinkCableManager invalid;
    LinkCableManager master;
    LinkCableManager slave;
    LinkCableManager third;

    Check(!invalid.Connect(0, 0), "reject link cable session zero");
    Check(invalid.GetStatus().mode == LinkCableModeFault,
        "invalid link cable session reports a fault");
    Check(master.Connect(session, 0), "connect first link peer");
    Check(slave.Connect(session, 0), "connect second link peer");
    Check(!third.Connect(session, 0), "reject a third link peer");
    Check(third.GetStatus().mode == LinkCableModeFault,
        "third peer reports a stable session-full fault");
    Check(master.GetStatus().peer_count == 2,
        "both link peers are visible");
    Check(master.GetStatus().local_peer_id != slave.GetStatus().local_peer_id,
        "link peers claim distinct slots");
    Check(master.IsPacingPeer(), "lowest link slot owns pacing");
    Check(!slave.IsPacingPeer(), "second link slot follows pacing");

    GB_LinkCableTransfer received;
    Check(!slave.PollTransfer(0, &received),
        "slave primes its remote generation cursor");

    slave.PublishPortState(0, 0x3C, 0xFC);
    slave.Synchronize(0, LINK_CABLE_FAST_SYNC_CYCLES);
    master.PublishPortState(64, 0xA5, 0xFF);

    u8 incoming = 0xFF;
    master.StartTransfer(64, 80, 16, 0xA5, 1, &incoming);
    Check(incoming == 0x3C, "master snapshots the armed slave byte");
    Check(slave.PollTransfer(0, &received),
        "slave receives the targeted transfer descriptor");
    Check(received.request_cycle == 0 && received.first_shift_cycle == 16 &&
        received.bit_cycles == 16 && received.transfer_id == 1 &&
        received.incoming_byte == 0xA5 && received.local_byte == 0x3C,
        "bus cycles convert into the slave local cycle domain");
    Check(!slave.PollTransfer(0, &received),
        "transfer descriptor is delivered exactly once");

    slave.PublishPortState(0, 0x66, 0x7C);
    slave.Synchronize(0, LINK_CABLE_FAST_SYNC_CYCLES);
    incoming = 0x00;
    master.StartTransfer(64, 80, 16, 0x99, 2, &incoming);
    Check(incoming == 0xFF, "unarmed peer supplies disconnected input");
    Check(!slave.PollTransfer(0, &received),
        "unarmed descriptor is consumed without delivery");

    slave.PublishPortState(0, 0x55, 0xFF);
    slave.Synchronize(0, LINK_CABLE_FAST_SYNC_CYCLES);
    incoming = 0x00;
    master.StartTransfer(64, 80, 16, 0xAA, 3, &incoming);
    Check(incoming == 0xFF,
        "simultaneous internal clocks use disconnected input");
    Check(master.GetStatus().clock_conflicts == 1,
        "simultaneous internal clocks increment the conflict metric");

    LinkCableStatus master_status = master.GetStatus();
    Check(master_status.transfers_published == 3,
        "one immutable descriptor is published per peer-visible byte");
    Check(master_status.transfers_unarmed == 2,
        "unarmed and conflicting peers are counted");
    Check(master_status.states_published == 1,
        "master state publication is counted");

    master.Stop();
    Check(slave.GetStatus().peer_count == 1,
        "stopped peer releases its slot");
    Check(slave.IsPacingPeer(), "remaining peer takes over pacing");

    LinkCableManager replacement;
    Check(replacement.Connect(session, 1000),
        "released slot can be claimed with a new generation");
    Check(!slave.PollTransfer(0, &received),
        "new generation does not replay old descriptors");

    replacement.Stop();
    slave.Stop();
    third.Stop();
}

static void TestSessionIsolation()
{
    LinkCableManager first;
    LinkCableManager second;

    Check(first.Connect(TestSession(1), 0), "connect isolated session one");
    Check(second.Connect(TestSession(2), 0), "connect isolated session two");
    Check(first.GetStatus().peer_count == 1 &&
        second.GetStatus().peer_count == 1,
        "different sessions remain isolated");

    first.Stop();
    second.Stop();
}

static void TestDelayedIdlePairing()
{
    u8 session = TestSession(5);
    LinkCableManager first;
    LinkCableManager second;

    Check(first.Connect(session, 0), "connect first delayed GUI peer");
    first.PublishPortState(0, 0xFF, 0x7E);
    std::this_thread::sleep_for(std::chrono::microseconds(
        LINK_CABLE_DETACH_US + 100000));
    Check(second.Connect(session, 0), "connect second delayed GUI peer");

    LinkCableStatus first_status = first.GetStatus();
    LinkCableStatus second_status = second.GetStatus();
    Check(first_status.peer_count == 2 && second_status.peer_count == 2,
        "idle-lease policy preserves a waiting GUI peer");
    Check(first_status.local_peer_id != second_status.local_peer_id,
        "delayed GUI peers claim distinct slots");

    first.Stop();
    second.Stop();
}

static void TestStalePeerRecovery()
{
    u8 session = TestSession(3);
    LinkCableManager survivor;
    LinkCableManager stale;

    Check(survivor.Connect(session, 0), "connect barrier survivor");
    Check(stale.Connect(session, 0), "connect stale barrier peer");

    std::chrono::steady_clock::time_point wall_start =
        std::chrono::steady_clock::now();
    clock_t cpu_start = clock();
    survivor.Synchronize(1024, LINK_CABLE_MAX_PROMISE_CYCLES);
    u64 wall_us = (u64)std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - wall_start).count();
    u64 cpu_us = (u64)(clock() - cpu_start) * 1000000ULL /
        CLOCKS_PER_SEC;

    LinkCableStatus status = survivor.GetStatus();
    Check(status.peer_count == 1, "stale barrier peer is detached");
    Check(status.peer_detaches == 1, "stale detach is counted");
    Check(wall_us >= LINK_CABLE_DETACH_US,
        "stale peer remains leased until its deadline");
    Check(wall_us < 1500000, "stale barrier wait is bounded");
    Check(cpu_us * 2 < wall_us,
        "stale barrier wait does not spin the host CPU");

    stale.Stop();
    survivor.Stop();
}

static void TestTwoProcesses()
{
#if defined(_WIN32)
    printf("Two-process link cable test skipped on Windows test host\n");
#else
    int ready_pipe[2];
    int ack_pipe[2];
    Check(pipe(ready_pipe) == 0, "create child ready pipe");
    Check(pipe(ack_pipe) == 0, "create child acknowledgement pipe");
    u8 session = TestSession(4);

    pid_t slave_pid = fork();
    Check(slave_pid >= 0, "fork link slave process");

    if (slave_pid == 0)
    {
        close(ready_pipe[0]);
        close(ack_pipe[0]);

        LinkCableManager slave;
        if (!slave.Connect(session, 0))
            _exit(10);

        slave.PublishPortState(0, 0x3C, 0xFC);
        slave.Synchronize(0, LINK_CABLE_FAST_SYNC_CYCLES);

        char ready = 1;
        if (write(ready_pipe[1], &ready, 1) != 1)
            _exit(11);

        GB_LinkCableTransfer transfer;
        bool received = false;
        u64 local_cycle = 0;

        for (int i = 0; i < 1000 && !received; i++)
        {
            received = slave.PollTransfer(local_cycle, &transfer);
            slave.Synchronize(local_cycle, LINK_CABLE_FAST_SYNC_CYCLES);
            local_cycle += LINK_CABLE_FAST_SYNC_CYCLES;

            if (!received)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (!received || transfer.incoming_byte != 0xA5 ||
            transfer.local_byte != 0x3C || transfer.bit_cycles != 16)
        {
            _exit(12);
        }

        char acknowledged = 1;
        if (write(ack_pipe[1], &acknowledged, 1) != 1)
            _exit(13);

        slave.Stop();
        _exit(0);
    }

    close(ready_pipe[1]);
    char ready = 0;
    Check(read(ready_pipe[0], &ready, 1) == 1 && ready == 1,
        "slave process attached and armed");

    pid_t master_pid = fork();
    Check(master_pid >= 0, "fork link master process");

    if (master_pid == 0)
    {
        close(ack_pipe[1]);

        LinkCableManager master;
        if (!master.Connect(session, 0))
            _exit(20);

        master.PublishPortState(0, 0xA5, 0xFF);
        u8 incoming = 0xFF;
        master.StartTransfer(0, 16, 16, 0xA5, 1, &incoming);
        if (incoming != 0x3C)
            _exit(21);

        char acknowledged = 0;
        if (read(ack_pipe[0], &acknowledged, 1) != 1 || acknowledged != 1)
            _exit(22);

        master.Stop();
        _exit(0);
    }

    close(ack_pipe[0]);
    close(ack_pipe[1]);
    close(ready_pipe[0]);

    int slave_status = 0;
    int master_status = 0;
    Check(waitpid(slave_pid, &slave_status, 0) == slave_pid,
        "wait for link slave process");
    Check(waitpid(master_pid, &master_status, 0) == master_pid,
        "wait for link master process");
    Check(WIFEXITED(slave_status) && WEXITSTATUS(slave_status) == 0,
        "slave process completed byte delivery");
    Check(WIFEXITED(master_status) && WEXITSTATUS(master_status) == 0,
        "master process completed byte snapshot");
#endif
}

int main()
{
    TestHelpers();
    TestAttachAndTransfer();
    TestSessionIsolation();
    TestDelayedIdlePairing();
    TestStalePeerRecovery();
    TestTwoProcesses();

    printf("Gearboy link cable manager tests passed\n");
    return 0;
}
