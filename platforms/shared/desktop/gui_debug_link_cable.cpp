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

#define GUI_DEBUG_LINK_CABLE_IMPORT
#include "gui_debug_link_cable.h"

#include "imgui.h"
#include "gearboy.h"
#include "Memory.h"
#include "Processor.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "gui_debug_widgets.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

static void serial_write_callback(u16 address, u8 value, void* user_data);
static void draw_byte_value(const char* label, u8 value);
static void draw_metric(const char* label, u64 value);
static void draw_metric_pair(const char* label, u64 first, u64 second);
static void draw_hardware_tab(GearboyCore* core, Memory* memory, Processor* processor);
static void draw_transport_tab(GearboyCore* core, Processor* processor);

void gui_debug_window_link_cable(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(100, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(266, 438), ImGuiCond_FirstUseEver);
    ImGui::Begin("Link Cable", &config_debug.show_link_cable);

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();
    Processor* processor = core->GetProcessor();

    ImGui::PushFont(gui_default_font);
    draw_hardware_tab(core, memory, processor);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_link_cable_transport(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(430, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(304, 550), ImGuiCond_FirstUseEver);
    ImGui::Begin("Link Cable (Transport)", &config_debug.show_link_cable_transport);

    GearboyCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();

    ImGui::PushFont(gui_default_font);
    draw_transport_tab(core, processor);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

static void serial_write_callback(u16 address, u8 value, void* user_data)
{
    Memory* memory = (Memory*)user_data;
    memory->Write(address, value);
}

static void draw_byte_value(const char* label, u8 value)
{
    ImGui::TextColored(violet, "%s", label); ImGui::SameLine();
    ImGui::Text("$%02X ", value); ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(value));
}

static void draw_metric(const char* label, u64 value)
{
    ImGui::TextColored(violet, "%s", label); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)value);
}

static void draw_metric_pair(const char* label, u64 first, u64 second)
{
    ImGui::TextColored(violet, "%s", label); ImGui::SameLine();
    ImGui::TextColored(white, "%llu / %llu", (unsigned long long)first, (unsigned long long)second);
}

static void draw_hardware_tab(GearboyCore* core, Memory* memory, Processor* processor)
{
    u8 sb = memory->Retrieve(0xFF01);
    u8 sc = memory->Retrieve(0xFF02);
    Processor::SerialState serial;
    processor->GetSerialState(serial);

    bool start_requested = (sc & 0x80) != 0;
    bool internal_clock = serial.transfer_active ? serial.internal_clock : ((sc & 0x01) != 0);
    bool fast_clock = core->IsCGB() && ((sc & 0x02) != 0);

    const char* mode = "IDLE";
    ImVec4 mode_color = gray;

    if (serial.restore_pending)
    {
        mode = "RESTORE PENDING";
        mode_color = yellow;
    }
    else if (serial.transfer_active)
    {
        mode = serial.internal_clock ? "INTERNAL ACTIVE" :
            "EXTERNAL ACTIVE";
        mode_color = green;
    }
    else if (serial.waiting_external)
    {
        mode = "EXTERNAL WAIT";
        mode_color = yellow;
    }

    ImGui::TextColored(magenta, "SERIAL REGISTERS:");

    EditableRegister8("SB", " $FF01", 0xFF01, sb, serial_write_callback, memory);
    EditableRegister8("SC", " $FF02", 0xFF02, sc, serial_write_callback, memory);

    ImGui::TextColored(violet, " START REQUEST  "); ImGui::SameLine();
    ImGui::TextColored(start_requested ? green : gray, "%s", start_requested ? "YES" : "NO");
    ImGui::TextColored(violet, " CLOCK SOURCE   "); ImGui::SameLine();
    ImGui::TextColored(internal_clock ? green : cyan, "%s", internal_clock ? "INTERNAL" : "EXTERNAL");
    ImGui::TextColored(violet, " CLOCK SPEED    "); ImGui::SameLine();
    ImGui::TextColored(fast_clock ? yellow : white, "%s", fast_clock ? "CGB FAST" : "NORMAL");
    ImGui::TextColored(violet, " CPU SPEED      "); ImGui::SameLine();
    ImGui::TextColored(processor->CGBSpeed() ? yellow : white, "%s", processor->CGBSpeed() ? "DOUBLE" : "NORMAL");
    ImGui::TextColored(violet, " SYSTEM         "); ImGui::SameLine();
    ImGui::TextColored(white, "%s", core->IsCGB() ? "CGB" : "DMG");
    ImGui::TextColored(violet, " CABLE          "); ImGui::SameLine();
    ImGui::TextColored(core->IsLinkCableConnected() ? green : gray, "%s", core->IsLinkCableConnected() ? "CONNECTED" : "DISCONNECTED");

    ImGui::Separator();
    ImGui::TextColored(magenta, "TRANSFER ENGINE:");

    ImGui::TextColored(violet, " MODE           "); ImGui::SameLine();
    ImGui::TextColored(mode_color, "%s", mode);

    ImGui::TextColored(violet, " BITS SHIFTED   "); ImGui::SameLine();

    if (serial.transfer_active || serial.waiting_external)
    {
        int bits = CLAMP(serial.bit_index, 0, 8);
        ImGui::TextColored(white, "%d / 8 bits", bits);
    }
    else
    {
        ImGui::TextColored(gray, "IDLE");
    }

    ImGui::TextColored(violet, " BIT PHASE      "); ImGui::SameLine();
    if (serial.transfer_active && serial.bit_cycles > 0)
    {
        ImGui::TextColored(white, "%d / %u cycles", serial.cycles, serial.bit_cycles);
    }
    else if (serial.waiting_external)
    {
        ImGui::TextColored(yellow, "WAITING FOR CLOCK");
    }
    else
    {
        ImGui::TextColored(gray, "-");
    }

    draw_byte_value(" OUTGOING BYTE  ", serial.outgoing_byte);
    draw_byte_value(" INCOMING BYTE  ", serial.incoming_byte);

    ImGui::TextColored(violet, " TRANSFER ID    "); ImGui::SameLine();
    ImGui::TextColored(white, "%u", serial.transfer_id);
    ImGui::TextColored(violet, " LINK CYCLE     "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)core->GetLinkCableCycle());
    ImGui::TextColored(violet, " LAST REQUEST   "); ImGui::SameLine();

    if (serial.transfer_id > 0)
    {
        ImGui::TextColored(white, "%llu", (unsigned long long)serial.request_cycle);
    }
    else
    {
        ImGui::TextColored(gray, "-");
    }

    ImGui::TextColored(violet, " NEXT EDGE      "); ImGui::SameLine();

    if (serial.transfer_active)
    {
        ImGui::TextColored(white, "%llu", (unsigned long long)serial.next_shift_cycle);
    }
    else
    {
        ImGui::TextColored(gray, "-");
    }

    ImGui::Separator();
    ImGui::TextColored(magenta, "SERIAL INTERRUPT:");

    bool irq_requested = (memory->Retrieve(0xFF0F) &
        Processor::Serial_Interrupt) != 0;
    bool irq_enabled = (memory->Retrieve(0xFFFF) &
        Processor::Serial_Interrupt) != 0;

    ImGui::TextColored(violet, " IF.3 REQUEST   "); ImGui::SameLine();
    ImGui::TextColored(irq_requested ? green : gray, "%s", irq_requested ? "YES" : "NO");
    ImGui::TextColored(violet, " IE.3 ENABLE    "); ImGui::SameLine();
    ImGui::TextColored(irq_enabled ? green : gray, "%s", irq_enabled ? "YES" : "NO");
    ImGui::TextColored(violet, " IRQ ASSERTED   "); ImGui::SameLine();
    ImGui::TextColored(irq_requested && irq_enabled ? green : gray, "%s", irq_requested && irq_enabled ? "YES" : "NO");
}

static void draw_transport_tab(GearboyCore* core, Processor* processor)
{
    LinkCableStatus link = emu_link_cable_get_status();

    const char* mode = "DISABLED";
    if (link.mode == LinkCableModeConnected)
        mode = "OK";
    else if (link.mode == LinkCableModeFault)
        mode = "FAULT";

    ImGui::TextColored(magenta, "SESSION:");

    ImGui::TextColored(violet, " CABLE          "); ImGui::SameLine();
    ImGui::TextColored(link.cable_connected ? green : red, "%s", link.cable_connected ? "CONNECTED" : "DISCONNECTED");
    ImGui::TextColored(violet, " STATUS         "); ImGui::SameLine();
    ImGui::TextColored(link.mode == LinkCableModeFault ? red : white, "%s", mode);
    ImGui::TextColored(violet, " SESSION        "); ImGui::SameLine();
    ImGui::TextColored(white, "%u", link.session);
    ImGui::TextColored(violet, " PEER           "); ImGui::SameLine();
    if (link.mode == LinkCableModeConnected)
        ImGui::TextColored(white, "%d / %d", link.local_peer_id, link.peer_count);
    else
        ImGui::TextColored(gray, "-");

    ImGui::TextColored(violet, " PACING         "); ImGui::SameLine();

    if (link.mode == LinkCableModeConnected)
    {
        ImGui::TextColored(link.pacing_peer ? green : cyan, "%s", link.pacing_peer ? "THIS PEER" : "REMOTE PEER");
    }
    else
    {
        ImGui::TextColored(gray, "-");
    }

    ImGui::TextColored(violet, " LINK CYCLE     "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)core->GetLinkCableCycle());
    ImGui::TextColored(violet, " PROMISE        "); ImGui::SameLine();
    ImGui::TextColored(white, "%u cycles", processor->GetLinkCablePromiseCycles(core->GetLinkCableCycle()));

    ImGui::Separator();
    ImGui::TextColored(magenta, "BYTE ACTIVITY:");

    draw_metric(" PORT STATES    ", link.states_published);
    draw_metric(" TX DESCRIPTORS ", link.transfers_published);
    draw_metric(" RX DESCRIPTORS ", link.transfers_delivered);
    draw_metric(" NO PEER        ", link.transfers_no_peer);
    draw_metric(" PEER UNARMED   ", link.transfers_unarmed);
    draw_metric(" CLOCK CONFLICT ", link.clock_conflicts);
    draw_metric(" LATE DESCRIPTOR", link.late_descriptors);

    ImGui::Separator();
    ImGui::TextColored(magenta, "SYNCHRONIZATION:");

    draw_metric(" SYNC CALLS     ", link.sync_calls);
    draw_metric_pair(" SNAPSHOT # / us", link.snapshot_waits, link.snapshot_wait_us);
    draw_metric_pair(" BARRIER # / us ", link.barrier_waits, link.barrier_wait_us);
    draw_metric_pair(" MAX WAIT/GAP us", link.barrier_wait_max_us, link.sync_gap_max_us);

    ImGui::TextColored(violet, " WAITS 1/10/50ms"); ImGui::SameLine();
    ImGui::TextColored(white, "%llu / %llu / %llu",
        (unsigned long long)link.barrier_wait_over_1ms,
        (unsigned long long)link.barrier_wait_over_10ms,
        (unsigned long long)link.barrier_wait_over_50ms);

    draw_metric_pair(" SPIN / SLEEP   ", link.spin_iterations, link.sleep_calls);

    ImGui::Separator();
    ImGui::TextColored(magenta, "RECOVERY:");

    draw_metric_pair(" DETACH / RECLAIM", link.peer_detaches, link.slot_reclaims);
    draw_metric(" ATTACHMENTS     ", link.attachments);
    draw_metric(" SEQ RETRIES     ", link.seqlock_retries);
    draw_metric_pair(" STATE/TX OVERRUN", link.state_ring_overruns, link.transfer_ring_overruns);

    if (link.mode == LinkCableModeFault)
    {
        ImGui::Separator();
        ImGui::TextColored(red, "%s", link.last_error);
    }

    ImGui::Separator();

    if (ImGui::Button("RESET METRICS"))
        emu_link_cable_reset_metrics();
}
