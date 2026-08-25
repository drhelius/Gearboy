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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "TraceLogger.h"
#include "Video.h"
#include "trace_logger_formatter.h"

bool g_mcp_stdio_mode = true;

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

class SerialTestCore
{
public:
    SerialTestCore(bool cgb, bool double_speed)
    {
        m_core.Init();

        u8 rom[0x8000];
        memset(rom, 0, sizeof(rom));
        rom[0x0100] = 0x3E; // LD A,$01
        rom[0x0101] = 0x01;
        rom[0x0102] = 0xE0; // LDH ($4D),A
        rom[0x0103] = 0x4D;
        rom[0x0104] = 0x10; // STOP
        rom[0x0105] = 0x00;
        rom[0x0106] = 0x18; // JR -2
        rom[0x0107] = 0xFE;
        rom[0x0143] = cgb ? 0x80 : 0x00;
        rom[0x0147] = 0x00;
        rom[0x0148] = 0x00;
        rom[0x0149] = 0x00;

        Check(m_core.LoadROMFromBuffer(rom, sizeof(rom), false),
            "load serial test ROM");

        if (double_speed)
        {
            Check(cgb, "double speed requires CGB mode");
            Processor* processor = m_core.GetProcessor();
            u64 cycle = 0;

            for (int i = 0; i < 20 && !processor->CGBSpeed(); i++)
            {
                u8 ticks = processor->RunFor(1);
                cycle += ticks;
                processor->UpdateTimers(ticks);
                processor->UpdateSerial(ticks, cycle);
            }

            Check(processor->CGBSpeed(), "switch test core to CGB double speed");
        }
    }

    GearboyCore* Core()
    {
        return &m_core;
    }

private:
    GearboyCore m_core;
};

struct FakeTransport
{
    u8 incoming;
    u32 start_calls;
    u32 state_calls;
    u32 bit_cycles;
    u64 request_cycle;
    u64 first_shift_cycle;
    bool poll_ready;
    GB_LinkCableTransfer transfer;
    u32 sync_calls;
    u32 promise_cycles;
};

static void FakeState(u64, u8, u8, void* user_data)
{
    FakeTransport* transport = (FakeTransport*)user_data;
    transport->state_calls++;
}

static void FakeStart(u64 request_cycle, u64 first_shift_cycle,
    u32 bit_cycles, u8, u32, u8* incoming_byte, void* user_data)
{
    FakeTransport* transport = (FakeTransport*)user_data;
    transport->start_calls++;
    transport->request_cycle = request_cycle;
    transport->first_shift_cycle = first_shift_cycle;
    transport->bit_cycles = bit_cycles;
    *incoming_byte = transport->incoming;
}

static bool FakePoll(u64, GB_LinkCableTransfer* transfer, void* user_data)
{
    FakeTransport* transport = (FakeTransport*)user_data;
    if (!transport->poll_ready)
        return false;

    *transfer = transport->transfer;
    transport->poll_ready = false;
    return true;
}

static void FakeSync(u64, u32 promise_cycles, void* user_data)
{
    FakeTransport* transport = (FakeTransport*)user_data;
    transport->sync_calls++;
    transport->promise_cycles = promise_cycles;
}

static void InstallFakeTransport(GearboyCore* core, FakeTransport* transport)
{
    memset(transport, 0, sizeof(*transport));
    transport->incoming = 0xFF;
    core->SetLinkCableCallbacks(FakeState, FakeStart, FakePoll, FakeSync,
        transport);
    core->SetLinkCableConnected(true);
}

static void TestRegisterMasks()
{
    SerialTestCore dmg(false, false);
    Memory* dmg_memory = dmg.Core()->GetMemory();

    dmg_memory->Write(0xFF02, 0x00);
    Check(dmg_memory->Retrieve(0xFF02) == 0x7E,
        "DMG SC zero write reads unused bits high");
    dmg_memory->Write(0xFF02, 0x83);
    Check(dmg_memory->Retrieve(0xFF02) == 0xFF,
        "DMG SC preserves only start and clock source");

    SerialTestCore cgb(true, false);
    Memory* cgb_memory = cgb.Core()->GetMemory();

    cgb_memory->Write(0xFF02, 0x00);
    Check(cgb_memory->Retrieve(0xFF02) == 0x7C,
        "CGB SC zero write reads unused bits high");
    cgb_memory->Write(0xFF02, 0x83);
    Check(cgb_memory->Retrieve(0xFF02) == 0xFF,
        "CGB SC preserves start, speed, and clock source");
}

static void TestInternalRate(bool cgb, bool double_speed, bool fast,
    u32 expected_bit_cycles)
{
    SerialTestCore test(cgb, double_speed);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    processor->ResetDIVCycles();
    memory->Load(0xFF0F, 0x00);
    memory->Write(0xFF01, 0x00);
    memory->Write(0xFF02, fast ? 0x83 : 0x81);

    const u64 start_cycle = 1000;
    processor->UpdateSerial(0, start_cycle);

    Check(transport.start_calls == 1, "internal transfer starts once");
    Check(transport.bit_cycles == expected_bit_cycles,
        "internal transfer selects expected bit period");
    Check(transport.first_shift_cycle == start_cycle + expected_bit_cycles,
        "first shift is one selected bit period after start");

    for (int edge = 1; edge <= 7; edge++)
    {
        processor->UpdateSerial(0,
            start_cycle + (u64)expected_bit_cycles * edge);
        Check((memory->Retrieve(0xFF02) & 0x80) != 0,
            "SC remains active through edge seven");
        Check((memory->Retrieve(0xFF0F) & Processor::Serial_Interrupt) == 0,
            "serial IRQ is not requested before edge eight");
    }

    processor->UpdateSerial(0,
        start_cycle + (u64)expected_bit_cycles * 8);

    Check(memory->Retrieve(0xFF01) == 0xFF,
        "disconnected internal transfer shifts in high bits");
    Check((memory->Retrieve(0xFF02) & 0x80) == 0,
        "SC clears on edge eight");
    Check((memory->Retrieve(0xFF0F) & Processor::Serial_Interrupt) != 0,
        "serial IRQ is requested on edge eight");
}

static void TestDividerSerialPhase(bool cgb, bool double_speed, bool fast,
    u8 phase_cycles, u32 expected_first_shift, u32 expected_bit_cycles)
{
    SerialTestCore test(cgb, double_speed);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    processor->ResetDIVCycles();
    processor->UpdateTimers(phase_cycles);

    const u64 start_cycle = 1000 + phase_cycles;
    memory->Write(0xFF01, 0x00);
    memory->Write(0xFF02, fast ? 0x83 : 0x81);
    processor->UpdateSerial(0, start_cycle);

    Check(transport.first_shift_cycle == start_cycle + expected_first_shift,
        "first serial shift follows the divider phase");
    Check(transport.bit_cycles == expected_bit_cycles,
        "divider phase does not change the serial bit period");

    processor->UpdateSerial(0, transport.first_shift_cycle - 1);
    Processor::SerialState state;
    processor->GetSerialState(state);
    Check(state.bit_index == 0,
        "serial transfer waits for the divider-derived first edge");

    processor->UpdateSerial(0, transport.first_shift_cycle);
    processor->GetSerialState(state);
    Check(state.bit_index == 1,
        "serial transfer shifts on the divider-derived first edge");
}

static void TestBootSerialPhase()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    processor->UpdateTimers(96);

    const u64 start_cycle = 96;
    memory->Write(0xFF01, 0x00);
    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, start_cycle);

    Check(transport.first_shift_cycle == 564,
        "DMG serial clock retains the skipped boot ROM divider phase");

    processor->ResetDIVCycles();
    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, start_cycle);

    Check(transport.first_shift_cycle == start_cycle + 512,
        "DIV reset clears the serial divider phase");
}

static void TestSerialDebugState()
{
    SerialTestCore test(true, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);
    transport.incoming = 0x96;

    memory->Write(0xFF01, 0xA5);
    memory->Write(0xFF02, 0x83);
    processor->UpdateSerial(0, 100);

    Processor::SerialState state;
    processor->GetSerialState(state);
    Check(state.transfer_active && state.internal_clock,
        "serial diagnostics report an active internal transfer");
    Check(!state.waiting_external && !state.restore_pending,
        "serial diagnostics distinguish active from waiting state");
    Check(!state.bytes_valid,
        "serial diagnostics do not expose an incomplete byte as a result");
    Check(state.bit_index == 0 && state.cycles == 0 &&
        state.bit_cycles == 16,
        "serial diagnostics expose initial bit timing");
    Check(state.outgoing_byte == 0xA5 && state.incoming_byte == 0x96,
        "serial diagnostics expose byte latches");
    Check(state.transfer_id == 1 && state.request_cycle == 100 &&
        state.next_shift_cycle == 116,
        "serial diagnostics expose transfer cycle anchors");

    processor->UpdateSerial(0, 140);
    processor->GetSerialState(state);
    Check(state.bit_index == 2 && state.cycles == 8 &&
        state.next_shift_cycle == 148,
        "serial diagnostics track bit progress and phase");

    processor->UpdateSerial(0, 228);
    processor->GetSerialState(state);
    Check(!state.transfer_active && state.bit_index == -1 &&
        state.bit_cycles == 0 && state.next_shift_cycle == 0,
        "serial diagnostics return to idle after edge eight");
    Check(state.bytes_valid,
        "serial diagnostics retain the completed byte pair");
}

static void TestExternalWait()
{
    SerialTestCore test(false, false);
    Memory* memory = test.Core()->GetMemory();
    Processor* processor = test.Core()->GetProcessor();

    memory->Load(0xFF0F, 0x00);
    memory->Write(0xFF01, 0x5A);
    memory->Write(0xFF02, 0x80);
    processor->UpdateSerial(0, 100);
    processor->UpdateSerial(0, 1000000);

    Check(memory->Retrieve(0xFF01) == 0x5A,
        "external clock waits without shifting");
    Check((memory->Retrieve(0xFF02) & 0x80) != 0,
        "external clock remains armed without a master");
    Check((memory->Retrieve(0xFF0F) & Processor::Serial_Interrupt) == 0,
        "external wait does not request a serial IRQ");
}

static void FormatSerialTrace(const GB_Trace_Entry& entry, char* buffer, size_t size)
{
    GB_Trace_Format_Options options = {};
    trace_logger_format_entry(entry, options, buffer, size);
}

static void TestSerialTraceFormatting()
{
    u64 master_cycle = 10;
    u64 link_cycle = 20;
    TraceLogger logger(&master_cycle, &link_cycle);
    Check(logger.GetLinkCableCycle() == 20,
        "serial trace logger exposes the monotonic link cycle");

    char buffer[GB_TRACE_FORMAT_BUFFER_SIZE];
    GB_Trace_Entry entry = {};
    entry.type = TRACE_SERIAL;
    entry.serial.event = TRACE_SERIAL_REG_WRITE;
    entry.serial.address = 0xFF01;
    entry.serial.value = 0x55;
    entry.serial.data = 0x55;
    entry.serial.control = 0xFE;
    entry.serial.link_cycle = 100;
    FormatSerialTrace(entry, buffer, sizeof(buffer));
    Check(strcmp(buffer,
        "  [SER]  WRITE SB($FF01) Raw:$55 Read:$55 SC:$FE Link:100") == 0,
        "serial trace identifies SB writes");

    entry.serial.address = 0xFF02;
    entry.serial.value = 0x80;
    entry.serial.link_cycle = 120;
    FormatSerialTrace(entry, buffer, sizeof(buffer));
    Check(strcmp(buffer,
        "  [SER]  WRITE SC($FF02) Raw:$80 Read:$FE SB:$55 Request:SET "
        "Clock:EXTERNAL Speed:NORMAL CPU:NORMAL System:DMG Link:120") == 0,
        "DMG SC trace reports an external request at normal speed");

    memset(&entry, 0, sizeof(entry));
    entry.type = TRACE_SERIAL;
    entry.serial.event = TRACE_SERIAL_TRANSFER_START;
    entry.serial.link_cycle = 200;
    entry.serial.request_cycle = 200;
    entry.serial.first_shift_cycle = 208;
    entry.serial.bit_cycles = 8;
    entry.serial.transfer_id = 7;
    entry.serial.internal_clock = 1;
    entry.serial.fast_clock = 1;
    entry.serial.cgb = 1;
    entry.serial.double_speed = 1;
    entry.serial.outgoing_byte = 0xA5;
    FormatSerialTrace(entry, buffer, sizeof(buffer));
    Check(strcmp(buffer,
        "  [SER]  TRANSFER START ID:7 TX:$A5 Clock:INTERNAL Speed:CGB_FAST "
        "CPU:DOUBLE System:CGB Period:8 Request:200 FirstEdge:208 Link:200") == 0,
        "serial start trace exposes transfer identity and phase timing");

    entry.serial.event = TRACE_SERIAL_TRANSFER_END;
    entry.serial.data = 0x3C;
    entry.serial.link_cycle = 264;
    FormatSerialTrace(entry, buffer, sizeof(buffer));
    Check(strcmp(buffer,
        "  [SER]  TRANSFER END ID:7 TX:$A5 RX:$3C Clock:INTERNAL Speed:CGB_FAST "
        "CPU:DOUBLE System:CGB Period:8 Link:264") == 0,
        "serial end trace distinguishes transmitted and received bytes");

    entry.serial.event = TRACE_SERIAL_IRQ_REQUEST;
    FormatSerialTrace(entry, buffer, sizeof(buffer));
    Check(strcmp(buffer,
        "  [SER]  IRQ REQUEST ID:7 RX:$3C Link:264") == 0,
        "serial IRQ trace retains transfer correlation");
}

#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
static void TestSerialTraceEvents()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    TraceLogger* logger = core->GetTraceLogger();
    logger->SetEnabledFlags(TRACE_FLAG_SERIAL);
    logger->SetEventFilter(TRACE_SERIAL, TRACE_SERIAL_FILTER_ALL);

    memory->Write(0xFF01, 0x55);
    memory->Write(0xFF02, 0x80);
    processor->UpdateSerial(0, 100);

    Check(logger->GetCount() == 2,
        "external clock arming logs register writes without a transfer start");
    Check(logger->GetEntry(0).serial.address == 0xFF01 &&
        logger->GetEntry(1).serial.address == 0xFF02,
        "serial register events retain their addresses");

    memory->Write(0xFF01, 0xA5);
    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, 200);

    Check(logger->GetCount() == 5 &&
        logger->GetEntry(4).serial.event == TRACE_SERIAL_TRANSFER_START,
        "internal clock activation emits one real transfer start");
    const GB_Trace_Entry& start = logger->GetEntry(4);
    Check(start.serial.link_cycle == 200 && start.serial.transfer_id == 1 &&
        start.serial.outgoing_byte == 0xA5 && start.serial.bit_cycles == 512,
        "transfer start event captures link timing and transmitted byte");

    Processor::SerialState state;
    processor->GetSerialState(state);
    processor->UpdateSerial(0,
        state.next_shift_cycle + (u64)state.bit_cycles * 7);

    Check(logger->GetCount() == 7 &&
        logger->GetEntry(5).serial.event == TRACE_SERIAL_TRANSFER_END &&
        logger->GetEntry(6).serial.event == TRACE_SERIAL_IRQ_REQUEST,
        "completed transfer emits correlated end and serial IRQ events");
}
#endif

static void TestRestartAndAbort()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    processor->ResetDIVCycles();
    memory->Load(0xFF0F, 0x00);
    memory->Write(0xFF01, 0x00);
    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, 100);
    processor->UpdateSerial(0, 612);
    Check(memory->Retrieve(0xFF01) == 0x01,
        "first pre-restart edge shifts once");

    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, 700);
    Check(transport.start_calls == 2,
        "same-value SC write restarts the transfer");

    processor->UpdateSerial(0, 1124);
    Check(memory->Retrieve(0xFF01) == 0x01,
        "restart replaces the old edge schedule");
    processor->UpdateSerial(0, 1212);
    Check(memory->Retrieve(0xFF01) == 0x03,
        "restarted transfer shifts on its new first edge");

    memory->Write(0xFF02, 0x01);
    processor->UpdateSerial(0, 1300);
    u8 aborted_data = memory->Retrieve(0xFF01);
    processor->UpdateSerial(0, 1000000);
    Check(memory->Retrieve(0xFF01) == aborted_data,
        "clearing SC start aborts an active transfer");
    Check((memory->Retrieve(0xFF0F) & Processor::Serial_Interrupt) == 0,
        "aborted transfer does not request a serial IRQ");
}

static void TestBitOrder()
{
    SerialTestCore test(true, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);
    transport.incoming = 0x96;

    memory->Write(0xFF01, 0xA5);
    memory->Write(0xFF02, 0x83);
    processor->UpdateSerial(0, 200);

    u8 expected = 0xA5;
    for (int edge = 1; edge <= 8; edge++)
    {
        expected = (u8)((expected << 1) |
            ((transport.incoming >> (8 - edge)) & 0x01));
        processor->UpdateSerial(0, 200 + (u64)16 * edge);
        Check(memory->Retrieve(0xFF01) == expected,
            "serial transfer shifts incoming byte MSB first");
    }

    Check(memory->Retrieve(0xFF01) == 0x96,
        "serial transfer receives the complete incoming byte");
}

static void TestLateExternalDescriptor()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    memory->Load(0xFF0F, 0x00);
    memory->Write(0xFF01, 0x3C);
    memory->Write(0xFF02, 0x80);

    transport.poll_ready = true;
    transport.transfer.request_cycle = 300;
    transport.transfer.first_shift_cycle = 316;
    transport.transfer.bit_cycles = 16;
    transport.transfer.transfer_id = 7;
    transport.transfer.incoming_byte = 0xA5;
    transport.transfer.local_byte = 0x3C;

    processor->UpdateSerial(0, 380);
    Check(memory->Retrieve(0xFF01) == 0x94,
        "late external descriptor catches up every due edge");
    Check((memory->Retrieve(0xFF02) & 0x80) != 0,
        "late descriptor remains active before edge eight");

    processor->UpdateSerial(0, 428);
    Check(memory->Retrieve(0xFF01) == 0xA5,
        "external transfer receives the master's byte");
    Check((memory->Retrieve(0xFF02) & 0x80) == 0,
        "late external transfer completes at its scheduled eighth edge");
}

struct PairTransport;

struct PairEndpoint
{
    PairTransport* transport;
    Memory* peer_memory;
    bool target;
};

struct PairTransport
{
    bool pending;
    GB_LinkCableTransfer transfer;
};

static void PairState(u64, u8, u8, void*)
{
}

static void PairStart(u64 request_cycle, u64 first_shift_cycle,
    u32 bit_cycles, u8 outgoing_byte, u32 transfer_id, u8* incoming_byte,
    void* user_data)
{
    PairEndpoint* endpoint = (PairEndpoint*)user_data;
    u8 peer_sc = endpoint->peer_memory->Retrieve(0xFF02);

    if ((peer_sc & 0x81) != 0x80)
    {
        *incoming_byte = 0xFF;
        return;
    }

    u8 peer_byte = endpoint->peer_memory->Retrieve(0xFF01);
    *incoming_byte = peer_byte;
    endpoint->transport->pending = true;
    endpoint->transport->transfer.request_cycle = request_cycle;
    endpoint->transport->transfer.first_shift_cycle = first_shift_cycle;
    endpoint->transport->transfer.bit_cycles = bit_cycles;
    endpoint->transport->transfer.transfer_id = transfer_id;
    endpoint->transport->transfer.incoming_byte = outgoing_byte;
    endpoint->transport->transfer.local_byte = peer_byte;
}

static bool PairPoll(u64, GB_LinkCableTransfer* transfer, void* user_data)
{
    PairEndpoint* endpoint = (PairEndpoint*)user_data;
    if (!endpoint->target || !endpoint->transport->pending)
        return false;

    *transfer = endpoint->transport->transfer;
    endpoint->transport->pending = false;
    return true;
}

static void PairSync(u64, u32, void*)
{
}

static void TestCrossCoreExchange()
{
    SerialTestCore master_test(true, false);
    SerialTestCore slave_test(false, false);
    GearboyCore* master = master_test.Core();
    GearboyCore* slave = slave_test.Core();
    PairTransport transport = {};
    PairEndpoint master_endpoint = {&transport, slave->GetMemory(), false};
    PairEndpoint slave_endpoint = {&transport, master->GetMemory(), true};

    master->SetLinkCableCallbacks(PairState, PairStart, PairPoll, PairSync,
        &master_endpoint);
    slave->SetLinkCableCallbacks(PairState, PairStart, PairPoll, PairSync,
        &slave_endpoint);
    master->SetLinkCableConnected(true);
    slave->SetLinkCableConnected(true);

    slave->GetMemory()->Load(0xFF0F, 0x00);
    master->GetMemory()->Load(0xFF0F, 0x00);
    slave->GetMemory()->Write(0xFF01, 0x3C);
    slave->GetMemory()->Write(0xFF02, 0x80);
    slave->GetProcessor()->UpdateSerial(0, 500);

    master->GetMemory()->Write(0xFF01, 0xA5);
    master->GetMemory()->Write(0xFF02, 0x83);
    master->GetProcessor()->UpdateSerial(0, 500);
    slave->GetProcessor()->UpdateSerial(0, 500);

    for (int edge = 1; edge <= 8; edge++)
    {
        u64 cycle = 500 + (u64)16 * edge;
        master->GetProcessor()->UpdateSerial(0, cycle);
        slave->GetProcessor()->UpdateSerial(0, cycle);
    }

    Check(master->GetMemory()->Retrieve(0xFF01) == 0x3C,
        "master receives the slave byte");
    Check(slave->GetMemory()->Retrieve(0xFF01) == 0xA5,
        "external DMG slave follows the CGB master's fast schedule");
    Check((master->GetMemory()->Retrieve(0xFF0F) &
        Processor::Serial_Interrupt) != 0,
        "master requests the serial IRQ");
    Check((slave->GetMemory()->Retrieve(0xFF0F) &
        Processor::Serial_Interrupt) != 0,
        "slave requests the serial IRQ on the shared edge");
}

static void TestSaveStateRuntimePolicy()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);

    memory->Load(0xFF0F, 0x00);
    memory->Write(0xFF01, 0x00);
    memory->Write(0xFF02, 0x81);
    processor->UpdateSerial(0, 100);
    processor->UpdateSerial(0, 612);
    processor->UpdateSerial(0, 1124);

    size_t state_size = 0;
    Check(core->SaveState(NULL, state_size, false) && state_size > 0,
        "query save-state size during a serial transfer");
    u8* state = new u8[state_size];
    size_t written = state_size;
    Check(core->SaveState(state, written, false) && written <= state_size,
        "save active serial state without transport fields");

    processor->UpdateSerial(0, 10000);
    Check(core->LoadState(state, written),
        "load a save state containing legacy serial fields");
    delete [] state;

    Check(!core->IsLinkCableConnected(),
        "loading a state disconnects runtime link transport");
    Check((memory->Retrieve(0xFF02) & 0x80) != 0,
        "loaded internal serial transfer remains active");

    processor->UpdateSerial(0, 2000);
    for (int edge = 1; edge <= 6; edge++)
        processor->UpdateSerial(0, 2000 + (u64)512 * edge);

    Check(memory->Retrieve(0xFF01) == 0xFF,
        "restored internal transfer resumes with disconnected input");
    Check((memory->Retrieve(0xFF02) & 0x80) == 0,
        "restored transfer completes after its remaining edges");
    Check((memory->Retrieve(0xFF0F) & Processor::Serial_Interrupt) != 0,
        "restored transfer requests its serial IRQ");
}

static void TestLinkSessionSurvivesReset()
{
    SerialTestCore test(false, false);
    GearboyCore* core = test.Core();
    FakeTransport transport;
    InstallFakeTransport(core, &transport);
    u64 trace_cycle = 0;
    TraceLogger trace_logger(&trace_cycle);
    core->GetVideo()->SetTraceLogger(&trace_logger);

    u16 frame_buffer[SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT];
    s16 audio_buffer[AUDIO_BUFFER_SIZE];
    int sample_count = 0;
    core->RunToVBlank(frame_buffer, audio_buffer, &sample_count, false,
        NULL, false);

    u64 link_cycle = core->GetLinkCableCycle();
    Check(link_cycle > 0, "link cable clock advances with emulation");
    Check(core->GetMasterClockCycles() > 0,
        "console master clock advances before reset");

    u32 state_calls = transport.state_calls;
    core->ResetROM(false);

    Check(core->IsLinkCableConnected(),
        "console reset preserves the active link cable");
    Check(core->GetLinkCableCycle() == link_cycle,
        "link cable clock remains monotonic across console reset");
    Check(core->GetMasterClockCycles() == 0,
        "console master clock still resets with the game");
    Check(transport.state_calls == state_calls + 1,
        "console reset republishes the idle serial state");

    sample_count = 0;
    core->RunToVBlank(frame_buffer, audio_buffer, &sample_count, false,
        NULL, false);
    Check(core->GetLinkCableCycle() > link_cycle,
        "link cable clock continues after console reset");

    core->GetVideo()->SetTraceLogger(NULL);
}

int main()
{
    TestRegisterMasks();
    TestInternalRate(false, false, false, 512);
    TestInternalRate(true, false, false, 512);
    TestInternalRate(true, false, true, 16);
    TestInternalRate(true, true, false, 256);
    TestInternalRate(true, true, true, 8);
    TestDividerSerialPhase(false, false, false, 64, 448, 512);
    TestDividerSerialPhase(true, false, true, 4, 12, 16);
    TestDividerSerialPhase(true, true, false, 2, 254, 256);
    TestDividerSerialPhase(true, true, true, 2, 6, 8);
    TestBootSerialPhase();
    TestSerialDebugState();
    TestExternalWait();
    TestSerialTraceFormatting();
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    TestSerialTraceEvents();
#endif
    TestRestartAndAbort();
    TestBitOrder();
    TestLateExternalDescriptor();
    TestCrossCoreExchange();
    TestSaveStateRuntimePolicy();
    TestLinkSessionSurvivesReset();

    printf("Gearboy serial tests passed\n");
    return 0;
}
