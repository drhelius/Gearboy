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

#include "mcp_debug_adapter.h"
#include "log.h"
#include "../utils.h"
#include "../emu.h"
#include "../gui.h"
#include "../gui_actions.h"
#include "../gui_debug_disassembler.h"
#include "../gui_debug_memory.h"
#include "../gui_debug_memeditor.h"
#include "../gui_debug_rewind.h"
#include "../config.h"
#include "../rewind.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

struct DisassemblerBookmark
{
    u16 address;
    char name[32];
};

static u32 GetMemoryAreaDisplayBase(int area)
{
    switch (area)
    {
        case MEMORY_EDITOR_ROM1:
            return 0x4000;
        case MEMORY_EDITOR_VRAM:
            return 0x8000;
        case MEMORY_EDITOR_RAM:
            return 0xA000;
        case MEMORY_EDITOR_WRAM0:
        case MEMORY_EDITOR_WRAM:
            return 0xC000;
        case MEMORY_EDITOR_WRAM1:
            return 0xD000;
        case MEMORY_EDITOR_OAM:
            return 0xFE00;
        case MEMORY_EDITOR_IO:
            return 0xFF00;
        case MEMORY_EDITOR_HIRAM:
            return 0xFF80;
        default:
            return 0x0000;
    }
}

static bool NormalizeMemoryAreaAddress(const MemoryAreaInfo& info, u32 display_base, u32 address, u32* offset)
{
    if (!IsValidPointer(offset) || !IsValidPointer(info.data) || info.size == 0)
        return false;

    u64 display_start = display_base;
    u64 display_end = display_start + info.size;

    if ((u64)address >= display_start && (u64)address < display_end)
    {
        *offset = address - display_base;
        return true;
    }

    if (address < info.size)
    {
        *offset = address;
        return true;
    }

    return false;
}

static bool MemoryAreaContainsDisplayAddress(const MemoryAreaInfo& info, u32 display_base, u32 address)
{
    if (!IsValidPointer(info.data) || info.size == 0)
        return false;

    u64 display_start = display_base;
    u64 display_end = display_start + info.size;

    return (u64)address >= display_start && (u64)address < display_end;
}

void DebugAdapter::Pause()
{
    emu_debug_break();
}

void DebugAdapter::Resume()
{
    emu_debug_continue();
}

void DebugAdapter::StepInto()
{
    emu_debug_step_into();
}

void DebugAdapter::StepOver()
{
    emu_debug_step_over();
}

void DebugAdapter::StepOut()
{
    emu_debug_step_out();
}

void DebugAdapter::StepFrame()
{
    emu_debug_step_frame();
}

void DebugAdapter::Reset()
{
    emu_reset(false, Cartridge::CartridgeNotSupported, false);
}

json DebugAdapter::GetDebugStatus()
{
    json result;

    if (!m_core)
    {
        result["error"] = "Core not initialized";
        return result;
    }

    bool is_paused = emu_is_debug_idle();

    result["paused"] = is_paused;

    if (is_paused)
    {
        Processor* cpu = m_core->GetProcessor();
        u16 pc = cpu->GetState()->PC->GetValue();

        bool at_breakpoint = cpu->BreakpointHit();

        result["at_breakpoint"] = at_breakpoint;

        std::ostringstream pc_ss;
        pc_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << pc;
        result["pc"] = pc_ss.str();
    }
    else
    {
        result["at_breakpoint"] = false;
    }

    return result;
}

void DebugAdapter::SetBreakpoint(u16 address, int type, bool read, bool write, bool execute)
{
    Processor* cpu = m_core->GetProcessor();

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04X", address);

    if (type == Processor::GB_BREAKPOINT_TYPE_ROMRAM && execute && !read && !write)
    {
        cpu->AddBreakpoint(address);
    }
    else
    {
        cpu->AddBreakpoint(type, buffer, read, write, execute);
    }
}

void DebugAdapter::SetBreakpointRange(u16 start_address, u16 end_address, int type, bool read, bool write, bool execute)
{
    Processor* cpu = m_core->GetProcessor();

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04X-%04X", start_address, end_address);

    cpu->AddBreakpoint(type, buffer, read, write, execute);
}

void DebugAdapter::ClearBreakpointByAddress(u16 address, int type, u16 end_address)
{
    Processor* cpu = m_core->GetProcessor();
    std::vector<Processor::GB_Breakpoint>* breakpoints = cpu->GetBreakpoints();

    for (int i = (int)breakpoints->size() - 1; i >= 0; i--)
    {
        Processor::GB_Breakpoint& bp = (*breakpoints)[i];

        if (bp.type != type)
            continue;

        if (end_address > 0 && end_address >= address)
        {
            if (bp.range && bp.address1 == address && bp.address2 == end_address)
                breakpoints->erase(breakpoints->begin() + i);
        }
        else
        {
            if (!bp.range && bp.address1 == address)
                breakpoints->erase(breakpoints->begin() + i);
        }
    }
}

std::vector<BreakpointInfo> DebugAdapter::ListBreakpoints()
{
    std::vector<BreakpointInfo> result;
    Processor* cpu = m_core->GetProcessor();
    std::vector<Processor::GB_Breakpoint>* breakpoints = cpu->GetBreakpoints();

    for (const Processor::GB_Breakpoint& brk : *breakpoints)
    {
        BreakpointInfo info;
        info.enabled = brk.enabled;
        info.type = brk.type;
        info.address1 = brk.address1;
        info.address2 = brk.address2;
        info.read = brk.read;
        info.write = brk.write;
        info.execute = brk.execute;
        info.range = brk.range;
        info.type_name = GetBreakpointTypeName(brk.type);
        result.push_back(info);
    }

    return result;
}

RegistersSnapshot DebugAdapter::GetRegisters()
{
    Debug("[MCP] GetRegisters: start");

    Processor* cpu = m_core->GetProcessor();
    Processor::ProcessorState* state = cpu->GetState();

    Debug("[MCP] GetRegisters: creating snapshot");
    RegistersSnapshot snapshot;

    snapshot.AF = state->AF->GetValue();
    snapshot.BC = state->BC->GetValue();
    snapshot.DE = state->DE->GetValue();
    snapshot.HL = state->HL->GetValue();
    snapshot.SP = state->SP->GetValue();
    snapshot.PC = state->PC->GetValue();
    snapshot.IME = *state->IME;
    snapshot.Halt = *state->Halt;
    snapshot.DoubleSpeed = cpu->CGBSpeed();

    Debug("[MCP] GetRegisters: done (PC=%04X)", snapshot.PC);
    return snapshot;
}

void DebugAdapter::SetRegister(const std::string& name, u32 value)
{
    Processor* cpu = m_core->GetProcessor();
    Processor::ProcessorState* state = cpu->GetState();

    if (name == "AF")
        state->AF->SetValue((u16)value);
    else if (name == "BC")
        state->BC->SetValue((u16)value);
    else if (name == "DE")
        state->DE->SetValue((u16)value);
    else if (name == "HL")
        state->HL->SetValue((u16)value);
    else if (name == "SP")
        state->SP->SetValue((u16)value);
    else if (name == "PC")
        state->PC->SetValue((u16)value);
    else if (name == "A")
        state->AF->SetHigh((u8)value);
    else if (name == "F")
        state->AF->SetLow((u8)value);
    else if (name == "B")
        state->BC->SetHigh((u8)value);
    else if (name == "C")
        state->BC->SetLow((u8)value);
    else if (name == "D")
        state->DE->SetHigh((u8)value);
    else if (name == "E")
        state->DE->SetLow((u8)value);
    else if (name == "H")
        state->HL->SetHigh((u8)value);
    else if (name == "L")
        state->HL->SetLow((u8)value);
}

std::vector<MemoryAreaInfo> DebugAdapter::ListMemoryAreas()
{
    std::vector<MemoryAreaInfo> result;

    for (int i = 0; i < MCP_MEMORY_AREA_MAX; i++)
    {
        MemoryAreaInfo info = GetMemoryAreaInfo(i);
        if (info.data != NULL && info.size > 0)
        {
            result.push_back(info);
        }
    }

    return result;
}

std::vector<u8> DebugAdapter::ReadMemoryArea(int area, u32 offset, size_t size)
{
    std::vector<u8> result;
    MemoryAreaInfo info = GetMemoryAreaInfo(area);

    if (info.data == NULL || offset >= info.size)
        return result;

    u32 bytes_to_read = (u32)size;
    if (offset + bytes_to_read > info.size)
        bytes_to_read = info.size - offset;

    for (u32 i = 0; i < bytes_to_read; i++)
    {
        result.push_back(info.data[offset + i]);
    }

    return result;
}

void DebugAdapter::WriteMemoryArea(int area, u32 offset, const std::vector<u8>& data)
{
    MemoryAreaInfo info = GetMemoryAreaInfo(area);

    if (info.data == NULL || offset >= info.size)
        return;

    for (size_t i = 0; i < data.size() && (offset + i) < info.size; i++)
    {
        info.data[offset + i] = data[i];
    }
}

std::vector<DisasmLine> DebugAdapter::GetDisassembly(u16 start_address, u16 end_address, int bank, bool resolve_symbols)
{
    std::vector<DisasmLine> result;
    Memory* memory = m_core->GetMemory();

    bool use_explicit_bank = (bank >= 0 && bank <= 0xFF);

    // Scan backwards to find any instruction that might span into our range
    u16 scan_start = start_address;
    const int MAX_INSTRUCTION_SIZE = 3; // SM83 max instruction size is 3 bytes

    for (int lookback = 1; lookback < MAX_INSTRUCTION_SIZE && scan_start > 0; lookback++)
    {
        u16 check_addr = start_address - lookback;

        if (use_explicit_bank)
        {
            u16 start_offset = start_address & 0x3FFF;
            if (lookback > start_offset)
                break;
            check_addr = (start_address & 0xC000) | (start_offset - lookback);
        }

        GB_Disassembler_Record* record = NULL;

        if (use_explicit_bank)
        {
            record = memory->GetDisassemblerRecord(check_addr, (u8)bank);
        }
        else
        {
            record = memory->GetDisassemblerRecord(check_addr);
        }

        if (IsValidPointer(record) && record->name[0] != 0)
        {
            u16 instr_end = check_addr + record->size - 1;
            if (instr_end >= start_address)
            {
                scan_start = check_addr;
                break;
            }
        }
    }

    u32 addr = scan_start;

    while (addr <= (u32)end_address)
    {
        GB_Disassembler_Record* record = NULL;

        if (use_explicit_bank)
            record = memory->GetDisassemblerRecord((u16)addr, (u8)bank);
        else
            record = memory->GetDisassemblerRecord((u16)addr);

        if (IsValidPointer(record) && record->name[0] != 0)
        {
            DisasmLine line;
            line.address = (u16)addr;
            line.bank = record->bank;
            line.name = record->name;
            strip_color_tags(line.name);
            line.bytes = record->bytes;
            line.segment = record->segment;
            line.size = record->size;
            line.jump = record->jump;
            line.jump_address = record->jump_address;
            line.jump_bank = record->jump_bank;
            line.has_operand_address = record->has_operand_address;
            line.operand_address = record->operand_address;
            line.subroutine = record->subroutine;
            line.irq = record->irq;

            if (resolve_symbols)
            {
                std::string instr = line.name;
                if (!gui_debug_resolve_symbol(record, instr, "", ""))
                    gui_debug_resolve_label(record, instr, "", "");
                line.name = instr;
            }

            result.push_back(line);

            u32 next_addr;

            if (use_explicit_bank)
            {
                u16 offset_in_bank = (u16)addr & 0x3FFF;
                offset_in_bank += (u16)record->size;
                if (offset_in_bank >= 0x4000)
                {
                    break;
                }
                next_addr = ((u16)addr & 0xC000) | offset_in_bank;
            }
            else
            {
                next_addr = addr + (u32)record->size;
            }

            if ((record->size == 0) || (next_addr <= addr))
                next_addr = addr + 1;

            addr = next_addr;
        }
        else
        {
            addr++;
        }
    }

    return result;
}

const char* DebugAdapter::GetBreakpointTypeName(int type)
{
    switch (type)
    {
        case Processor::GB_BREAKPOINT_TYPE_ROMRAM:
            return "ROM/RAM";
        case Processor::GB_BREAKPOINT_TYPE_VRAM:
            return "VRAM";
        case Processor::GB_BREAKPOINT_TYPE_IO:
            return "IO";
        default:
            return "UNKNOWN";
    }
}

MemoryAreaInfo DebugAdapter::GetMemoryAreaInfo(int area)
{
    MemoryAreaInfo info;
    info.id = area;
    info.data = NULL;
    info.size = 0;

    Memory* memory = m_core->GetMemory();

    switch (area)
    {
        case MEMORY_EDITOR_ROM0:
            info.name = "ROM0";
            info.data = memory->GetROM0();
            info.size = 0x4000;
            break;
        case MEMORY_EDITOR_ROM1:
            info.name = "ROM1";
            info.data = memory->GetROM1();
            info.size = 0x4000;
            break;
        case MEMORY_EDITOR_VRAM:
            info.name = "VRAM";
            info.data = memory->GetVRAM();
            info.size = 0x2000;
            break;
        case MEMORY_EDITOR_RAM:
            info.name = "RAM";
            info.data = memory->GetRAM();
            info.size = 0x2000;
            break;
        case MEMORY_EDITOR_WRAM0:
            info.name = "WRAM0";
            info.data = memory->GetWRAM0();
            info.size = 0x1000;
            break;
        case MEMORY_EDITOR_WRAM1:
            info.name = "WRAM1";
            info.data = memory->GetWRAM1();
            info.size = 0x1000;
            break;
        case MEMORY_EDITOR_WRAM:
            info.name = "WRAM";
            info.data = memory->GetWRAM0();
            info.size = 0x2000;
            break;
        case MEMORY_EDITOR_OAM:
            info.name = "OAM";
            info.data = memory->GetMemoryMap() + 0xFE00;
            info.size = 0x00A0;
            break;
        case MEMORY_EDITOR_IO:
            info.name = "IO";
            info.data = memory->GetMemoryMap() + 0xFF00;
            info.size = 0x0080;
            break;
        case MEMORY_EDITOR_HIRAM:
            info.name = "HIRAM";
            info.data = memory->GetMemoryMap() + 0xFF80;
            info.size = 0x007F;
            break;
        case MEMORY_EDITOR_SGB_BORDER_TILES:
        {
            info.name = "SGB_TILES";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                const SGB::Border* border = sgb->GetBorder();
                info.data = (u8*)border->tiles;
                info.size = SGB_BORDER_TILE_DATA_SIZE;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_BORDER_MAP:
        {
            info.name = "SGB_MAP";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                const SGB::Border* border = sgb->GetBorder();
                info.data = (u8*)border->map;
                info.size = 32 * 32 * 2;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_BORDER_PAL:
        {
            info.name = "SGB_BPAL";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                const SGB::Border* border = sgb->GetBorder();
                info.data = (u8*)border->palette;
                info.size = 16 * 4 * 2;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_SYS_PAL:
        {
            info.name = "SGB_SPAL";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                info.data = (u8*)sgb->GetSystemPalettes();
                info.size = SGB_SYSTEM_PALETTE_COUNT * 4 * 2;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_ATTR_FILES:
        {
            info.name = "SGB_ATF";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                info.data = (u8*)sgb->GetAttributeFiles();
                info.size = SGB_ATF_COUNT * SGB_ATF_SIZE;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_ATTR_MAP:
        {
            info.name = "SGB_AMAP";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                info.data = (u8*)sgb->GetAttributeMap();
                info.size = SGB_ATTR_MAP_WIDTH * SGB_ATTR_MAP_HEIGHT;
            }
            break;
        }
        case MEMORY_EDITOR_SGB_EFF_PAL:
        {
            info.name = "SGB_EPAL";
            SGB* sgb = m_core->GetSGB();
            if (m_core->IsSGB())
            {
                info.data = (u8*)sgb->GetEffectivePalettes();
                info.size = 4 * 4 * 2;
            }
            break;
        }
        case MCP_MEMORY_AREA_VRAM0:
            info.name = "VRAM0";
            info.data = memory->GetVRAMBank0();
            info.size = 0x2000;
            break;
        case MCP_MEMORY_AREA_VRAM1:
            info.name = "VRAM1";
            info.data = memory->GetVRAMBank1();
            info.size = 0x2000;
            break;
        default:
            break;
    }

    return info;
}

json DebugAdapter::GetMediaInfo()
{
    json info;
    Cartridge* cart = m_core->GetCartridge();

    info["emulator"] = GEARBOY_TITLE;
    info["emulator_version"] = GEARBOY_VERSION;
    info["ready"] = cart->IsLoadedROM();
    info["file_path"] = cart->GetFilePath();
    info["file_name"] = cart->GetFileName();
    info["file_directory"] = cart->GetFileDirectory();
    info["rom_name"] = cart->GetName();

    info["rom_size"] = cart->GetROMSize();
    info["ram_size"] = cart->GetRAMSize();
    info["rom_bank_count"] = cart->GetROMBankCount();
    info["ram_bank_count"] = cart->GetRAMBankCount();
    info["has_battery"] = cart->HasBattery();
    info["is_cgb"] = cart->IsCGB();
    info["is_sgb"] = cart->IsSGB();
    info["is_rtc"] = cart->IsRTCPresent();
    info["is_rumble"] = cart->IsRumblePresent();
    info["is_mbc30"] = cart->IsMBC30();
    info["version"] = cart->GetVersion();
    info["valid_rom"] = cart->IsValidROM();

    Cartridge::CartridgeTypes type = cart->GetType();
    const char* type_names[] = {
        "ROM Only", "MBC1", "MBC2", "MBC3",
        "MBC5", "MBC1 Multi", "HuC1", "HuC3",
        "MMM01", "Camera", "MBC7", "TAMA5",
        "Not Supported"
    };
    int type_idx = (int)type;
    if (type_idx >= 0 && type_idx < (int)(sizeof(type_names) / sizeof(type_names[0])))
        info["cartridge_type"] = type_names[type_idx];
    else
        info["cartridge_type"] = "Unknown";

    if (m_core->IsCGB())
        info["system"] = m_core->IsGBA() ? "Game Boy Color (GBA mode)" : "Game Boy Color";
    else
        info["system"] = "Game Boy";

    return info;
}

json DebugAdapter::GetCPUStatus()
{
    json status;
    Processor* cpu = m_core->GetProcessor();
    Processor::ProcessorState* state = cpu->GetState();
    Memory* memory = m_core->GetMemory();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // Register pairs
    ss << std::setw(4) << state->AF->GetValue();
    status["AF"] = ss.str(); ss.str("");

    ss << std::setw(4) << state->BC->GetValue();
    status["BC"] = ss.str(); ss.str("");

    ss << std::setw(4) << state->DE->GetValue();
    status["DE"] = ss.str(); ss.str("");

    ss << std::setw(4) << state->HL->GetValue();
    status["HL"] = ss.str(); ss.str("");

    ss << std::setw(4) << state->SP->GetValue();
    status["SP"] = ss.str(); ss.str("");

    ss << std::setw(4) << state->PC->GetValue();
    status["PC"] = ss.str(); ss.str("");

    // Individual 8-bit registers
    u8 a = state->AF->GetHigh();
    u8 f = state->AF->GetLow();

    ss << std::setw(2) << (int)a;
    status["A"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)f;
    status["F"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->BC->GetHigh();
    status["B"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->BC->GetLow();
    status["C"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->DE->GetHigh();
    status["D"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->DE->GetLow();
    status["E"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->HL->GetHigh();
    status["H"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)state->HL->GetLow();
    status["L"] = ss.str(); ss.str("");

    // Flags decoded from F register (bits 7-4)
    status["flag_Z"] = (f & FLAG_ZERO) != 0;
    status["flag_N"] = (f & FLAG_SUB) != 0;
    status["flag_H"] = (f & FLAG_HALF) != 0;
    status["flag_C"] = (f & FLAG_CARRY) != 0;

    // Physical PC and bank
    ss << std::setw(6) << memory->GetPhysicalAddress(state->PC->GetValue());
    status["physical_PC"] = ss.str(); ss.str("");

    ss << std::setw(2) << (int)memory->GetBank(state->PC->GetValue());
    status["bank"] = ss.str(); ss.str("");

    // Interrupt state
    status["IME"] = *state->IME;
    status["Halt"] = *state->Halt;
    status["double_speed"] = cpu->CGBSpeed();

    // Interrupt registers
    u8 ie = memory->Retrieve(0xFFFF);
    u8 if_reg = memory->Retrieve(0xFF0F);
    ss << std::setw(2) << (int)ie;
    status["IE"] = ss.str(); ss.str("");
    ss << std::setw(2) << (int)if_reg;
    status["IF"] = ss.str(); ss.str("");

    return status;
}

json DebugAdapter::GetLCDRegisters()
{
    json result;
    Memory* memory = m_core->GetMemory();

    struct LCDReg {
        u16 addr;
        const char* name;
    };

    LCDReg lcd_regs[] = {
        {0xFF40, "LCDC"}, {0xFF41, "STAT"}, {0xFF42, "SCY"}, {0xFF43, "SCX"},
        {0xFF44, "LY"}, {0xFF45, "LYC"}, {0xFF46, "DMA"}, {0xFF47, "BGP"},
        {0xFF48, "OBP0"}, {0xFF49, "OBP1"}, {0xFF4A, "WY"}, {0xFF4B, "WX"}
    };

    json registers = json::array();
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (int i = 0; i < 12; i++)
    {
        json reg;
        u8 val = memory->Retrieve(lcd_regs[i].addr);
        ss << std::setw(2) << (int)val;
        reg["address"] = lcd_regs[i].addr;
        reg["name"] = lcd_regs[i].name;
        reg["value"] = ss.str(); ss.str("");
        registers.push_back(reg);
    }

    result["registers"] = registers;

    // Decode LCDC bit fields
    u8 lcdc = memory->Retrieve(0xFF40);
    json decoded;
    decoded["lcd_enable"] = (lcdc & 0x80) != 0;
    decoded["window_tile_map"] = (lcdc & 0x40) ? "9C00-9FFF" : "9800-9BFF";
    decoded["window_enable"] = (lcdc & 0x20) != 0;
    decoded["bg_window_tile_data"] = (lcdc & 0x10) ? "8000-8FFF" : "8800-97FF";
    decoded["bg_tile_map"] = (lcdc & 0x08) ? "9C00-9FFF" : "9800-9BFF";
    decoded["sprite_size"] = (lcdc & 0x04) ? "8x16" : "8x8";
    decoded["sprite_enable"] = (lcdc & 0x02) != 0;
    decoded["bg_window_enable"] = (lcdc & 0x01) != 0;

    // Decode STAT
    u8 stat = memory->Retrieve(0xFF41);
    decoded["stat_lyc_interrupt"] = (stat & 0x40) != 0;
    decoded["stat_oam_interrupt"] = (stat & 0x20) != 0;
    decoded["stat_vblank_interrupt"] = (stat & 0x10) != 0;
    decoded["stat_hblank_interrupt"] = (stat & 0x08) != 0;
    decoded["stat_lyc_match"] = (stat & 0x04) != 0;
    decoded["stat_mode"] = (int)(stat & 0x03);

    // Decode DMG palettes
    u8 bgp = memory->Retrieve(0xFF47);
    json bgp_decoded = json::array();
    for (int i = 0; i < 4; i++)
        bgp_decoded.push_back((bgp >> (i * 2)) & 0x03);
    decoded["bgp_colors"] = bgp_decoded;

    u8 obp0 = memory->Retrieve(0xFF48);
    json obp0_decoded = json::array();
    for (int i = 0; i < 4; i++)
        obp0_decoded.push_back((obp0 >> (i * 2)) & 0x03);
    decoded["obp0_colors"] = obp0_decoded;

    u8 obp1 = memory->Retrieve(0xFF49);
    json obp1_decoded = json::array();
    for (int i = 0; i < 4; i++)
        obp1_decoded.push_back((obp1 >> (i * 2)) & 0x03);
    decoded["obp1_colors"] = obp1_decoded;

    result["decoded"] = decoded;

    // CGB registers
    if (m_core->IsCGB())
    {
        json cgb = json::object();
        ss << std::setw(2) << (int)memory->Retrieve(0xFF4D);
        cgb["KEY1"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF4F);
        cgb["VBK"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF51);
        cgb["HDMA1"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF52);
        cgb["HDMA2"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF53);
        cgb["HDMA3"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF54);
        cgb["HDMA4"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF55);
        cgb["HDMA5"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF68);
        cgb["BCPS"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF69);
        cgb["BCPD"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF6A);
        cgb["OCPS"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF6B);
        cgb["OCPD"] = ss.str(); ss.str("");
        ss << std::setw(2) << (int)memory->Retrieve(0xFF70);
        cgb["SVBK"] = ss.str(); ss.str("");
        result["cgb_registers"] = cgb;
    }

    return result;
}

json DebugAdapter::GetLCDStatus()
{
    json status;
    Memory* memory = m_core->GetMemory();
    Video* video = m_core->GetVideo();

    status["screen_enabled"] = video->IsScreenEnabled();
    status["mode"] = video->GetCurrentStatusMode();
    status["ly"] = (int)memory->Retrieve(0xFF44);
    status["lyc"] = (int)memory->Retrieve(0xFF45);

    u8 stat = memory->Retrieve(0xFF41);
    status["lyc_match"] = (stat & 0x04) != 0;

    status["is_cgb"] = m_core->IsCGB();
    status["is_gba"] = m_core->IsGBA();
    status["double_speed"] = m_core->GetProcessor()->CGBSpeed();

    return status;
}

json DebugAdapter::GetAPUStatus()
{
    json status;
    Audio* audio = m_core->GetAudio();
    Gb_Apu* apu = audio->GetApu();

    gb_apu_state_t apu_state;
    apu->save_state(&apu_state);

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // Helper to read val_t (int or byte array depending on config)
    #if GB_APU_CUSTOM_STATE
    #define APU_VAL(x) (x)
    #else
    #define APU_VAL(x) ((int)(x)[0] | ((int)(x)[1] << 8) | ((int)(x)[2] << 16) | ((int)(x)[3] << 24))
    #endif

    json channels = json::array();

    // Channel 1: Square with sweep
    {
        json ch;
        ch["index"] = 0;
        ch["type"] = "square1";
        ch["enabled"] = APU_VAL(apu_state.enabled[0]) != 0;

        u8 nr10 = apu_state.regs[0x10 - 0x10];
        u8 nr11 = apu_state.regs[0x11 - 0x10];
        u8 nr12 = apu_state.regs[0x12 - 0x10];
        u8 nr13 = apu_state.regs[0x13 - 0x10];
        u8 nr14 = apu_state.regs[0x14 - 0x10];

        ch["dac"] = (nr12 & 0xF8) != 0;
        ch["volume"] = APU_VAL(apu_state.env_volume[0]);

        int duty = (nr11 >> 6) & 0x03;
        const char* duty_names[] = {"12.5%", "25%", "50%", "75%"};
        ch["duty"] = duty_names[duty];

        int freq = ((nr14 & 0x07) << 8) | nr13;
        ch["frequency_raw"] = freq;
        if (freq > 0)
            ch["frequency_hz"] = 131072.0 / (2048.0 - freq);
        else
            ch["frequency_hz"] = 0.0;

        ch["phase"] = APU_VAL(apu_state.phase[0]);
        ch["length_counter"] = APU_VAL(apu_state.length_ctr[0]);
        ch["length_enabled"] = (nr14 & 0x40) != 0;

        // Envelope
        ch["env_initial_volume"] = (nr12 >> 4) & 0x0F;
        ch["env_direction"] = (nr12 & 0x08) ? "up" : "down";
        ch["env_pace"] = nr12 & 0x07;

        // Sweep
        ch["sweep_pace"] = (nr10 >> 4) & 0x07;
        ch["sweep_direction"] = (nr10 & 0x08) ? "decrease" : "increase";
        ch["sweep_step"] = nr10 & 0x07;
        ch["sweep_enabled"] = APU_VAL(apu_state.sweep_enabled) != 0;

        channels.push_back(ch);
    }

    // Channel 2: Square
    {
        json ch;
        ch["index"] = 1;
        ch["type"] = "square2";
        ch["enabled"] = APU_VAL(apu_state.enabled[1]) != 0;

        u8 nr21 = apu_state.regs[0x16 - 0x10];
        u8 nr22 = apu_state.regs[0x17 - 0x10];
        u8 nr23 = apu_state.regs[0x18 - 0x10];
        u8 nr24 = apu_state.regs[0x19 - 0x10];

        ch["dac"] = (nr22 & 0xF8) != 0;
        ch["volume"] = APU_VAL(apu_state.env_volume[1]);

        int duty = (nr21 >> 6) & 0x03;
        const char* duty_names[] = {"12.5%", "25%", "50%", "75%"};
        ch["duty"] = duty_names[duty];

        int freq = ((nr24 & 0x07) << 8) | nr23;
        ch["frequency_raw"] = freq;
        if (freq > 0)
            ch["frequency_hz"] = 131072.0 / (2048.0 - freq);
        else
            ch["frequency_hz"] = 0.0;

        ch["phase"] = APU_VAL(apu_state.phase[1]);
        ch["length_counter"] = APU_VAL(apu_state.length_ctr[1]);
        ch["length_enabled"] = (nr24 & 0x40) != 0;

        ch["env_initial_volume"] = (nr22 >> 4) & 0x0F;
        ch["env_direction"] = (nr22 & 0x08) ? "up" : "down";
        ch["env_pace"] = nr22 & 0x07;

        channels.push_back(ch);
    }

    // Channel 3: Wave
    {
        json ch;
        ch["index"] = 2;
        ch["type"] = "wave";
        ch["enabled"] = APU_VAL(apu_state.enabled[2]) != 0;

        u8 nr30 = apu_state.regs[0x1A - 0x10];
        u8 nr32 = apu_state.regs[0x1C - 0x10];
        u8 nr33 = apu_state.regs[0x1D - 0x10];
        u8 nr34 = apu_state.regs[0x1E - 0x10];

        ch["dac"] = (nr30 & 0x80) != 0;

        int vol_shift = (nr32 >> 5) & 0x03;
        const char* vol_names[] = {"Mute", "100%", "50%", "25%"};
        ch["volume"] = vol_names[vol_shift];

        int freq = ((nr34 & 0x07) << 8) | nr33;
        ch["frequency_raw"] = freq;
        if (freq > 0)
            ch["frequency_hz"] = 65536.0 / (2048.0 - freq);
        else
            ch["frequency_hz"] = 0.0;

        ch["phase"] = APU_VAL(apu_state.phase[2]);
        ch["length_counter"] = APU_VAL(apu_state.length_ctr[2]);
        ch["length_enabled"] = (nr34 & 0x40) != 0;

        // Wave RAM
        json wave_ram = json::array();
        for (int i = 0; i < 16; i++)
        {
            ss << std::setw(2) << (int)apu_state.regs[0x30 - 0x10 + i];
            wave_ram.push_back(ss.str()); ss.str("");
        }
        ch["wave_ram"] = wave_ram;

        channels.push_back(ch);
    }

    // Channel 4: Noise
    {
        json ch;
        ch["index"] = 3;
        ch["type"] = "noise";
        ch["enabled"] = APU_VAL(apu_state.enabled[3]) != 0;

        u8 nr42 = apu_state.regs[0x21 - 0x10];
        u8 nr43 = apu_state.regs[0x22 - 0x10];
        u8 nr44 = apu_state.regs[0x23 - 0x10];

        ch["dac"] = (nr42 & 0xF8) != 0;
        ch["volume"] = APU_VAL(apu_state.env_volume[2]);

        int clock_shift = (nr43 >> 4) & 0x0F;
        bool lfsr_width = (nr43 & 0x08) != 0;
        int divisor_code = nr43 & 0x07;

        ch["clock_shift"] = clock_shift;
        ch["lfsr_width"] = lfsr_width ? "7-bit" : "15-bit";
        ch["divisor_code"] = divisor_code;

        static const int divisors[] = {8, 16, 32, 48, 64, 80, 96, 112};
        float freq_hz = 262144.0f / (float)divisors[divisor_code] / (float)(1 << clock_shift);
        ch["frequency_hz"] = freq_hz;

        ch["phase"] = APU_VAL(apu_state.phase[3]);
        ch["length_counter"] = APU_VAL(apu_state.length_ctr[3]);
        ch["length_enabled"] = (nr44 & 0x40) != 0;

        ch["env_initial_volume"] = (nr42 >> 4) & 0x0F;
        ch["env_direction"] = (nr42 & 0x08) ? "up" : "down";
        ch["env_pace"] = nr42 & 0x07;

        channels.push_back(ch);
    }

    #undef APU_VAL

    status["channels"] = channels;

    // Global audio registers
    u8 nr50 = apu_state.regs[0x24 - 0x10];
    u8 nr51 = apu_state.regs[0x25 - 0x10];
    u8 nr52 = apu_state.regs[0x26 - 0x10];

    status["master_enabled"] = (nr52 & 0x80) != 0;
    status["master_volume_left"] = (nr50 >> 4) & 0x07;
    status["master_volume_right"] = nr50 & 0x07;
    status["vin_left"] = (nr50 & 0x80) != 0;
    status["vin_right"] = (nr50 & 0x08) != 0;

    // Panning
    json panning;
    panning["ch1_left"] = (nr51 & 0x10) != 0;
    panning["ch1_right"] = (nr51 & 0x01) != 0;
    panning["ch2_left"] = (nr51 & 0x20) != 0;
    panning["ch2_right"] = (nr51 & 0x02) != 0;
    panning["ch3_left"] = (nr51 & 0x40) != 0;
    panning["ch3_right"] = (nr51 & 0x04) != 0;
    panning["ch4_left"] = (nr51 & 0x80) != 0;
    panning["ch4_right"] = (nr51 & 0x08) != 0;
    status["panning"] = panning;

    return status;
}

// Base64 encoding table
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const unsigned char* data, int size)
{
    std::string result;
    result.reserve(((size + 2) / 3) * 4);

    int i = 0;
    while (i < size)
    {
        unsigned char byte1 = data[i++];
        unsigned char byte2 = (i < size) ? data[i++] : 0;
        unsigned char byte3 = (i < size) ? data[i++] : 0;

        result.push_back(base64_chars[byte1 >> 2]);
        result.push_back(base64_chars[((byte1 & 0x03) << 4) | (byte2 >> 4)]);
        result.push_back((i > size + 1) ? '=' : base64_chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)]);
        result.push_back((i > size) ? '=' : base64_chars[byte3 & 0x3F]);
    }

    return result;
}

json DebugAdapter::GetScreenshot()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    GB_RuntimeInfo runtime;
    m_core->GetRuntimeInfo(runtime);

    unsigned char* png_buffer = NULL;
    int png_size = emu_get_screenshot_png(&png_buffer);

    if (png_size == 0 || !png_buffer)
    {
        result["error"] = "Failed to capture screenshot";
        return result;
    }

    std::string base64_png = base64_encode(png_buffer, png_size);
    free(png_buffer);

    result["__mcp_image"] = true;
    result["data"] = base64_png;
    result["mimeType"] = "image/png";
    result["width"] = runtime.screen_width;
    result["height"] = runtime.screen_height;

    return result;
}

json DebugAdapter::GetSGBStatus()
{
    json result;
    SGB* sgb = m_core->GetSGB();

    result["sgb_active"] = m_core->IsSGB();

    if (!m_core->IsSGB())
        return result;

    static const char* mask_names[] = {"disabled", "freeze", "black", "color0"};
    static const char* transfer_names[] = {"low_tiles", "high_tiles", "border_data", "palettes", "attributes"};

    int mask = sgb->GetMaskMode();
    result["mask_mode"] = mask;
    result["mask_mode_name"] = (mask >= 0 && mask <= 3) ? mask_names[mask] : "unknown";
    result["commands_disabled"] = sgb->AreCommandsDisabled();
    result["player_count"] = sgb->GetPlayerCount();
    result["current_player"] = sgb->GetCurrentPlayer();

    const u8* cmd = sgb->GetCommand();
    u8 cmdCode = cmd[0] >> 3;
    u8 cmdLen = cmd[0] & 7;
    result["last_command_code"] = cmdCode;
    result["last_command_length"] = cmdLen;
    result["command_write_index"] = sgb->GetCommandWriteIndex();
    result["ready_for_pulse"] = sgb->IsReadyForPulse();
    result["ready_for_write"] = sgb->IsReadyForWrite();
    result["ready_for_stop"] = sgb->IsReadyForStop();

    char cmd_hex[SGB_PACKET_SIZE * 3];
    for (int i = 0; i < SGB_PACKET_SIZE; i++)
        snprintf(cmd_hex + i * 3, 4, "%02X ", cmd[i]);
    cmd_hex[SGB_PACKET_SIZE * 3 - 1] = '\0';
    result["command_data"] = cmd_hex;

    u8 countdown = sgb->GetVRAMTransferCountdown();
    result["transfer_countdown"] = countdown;
    int dest = sgb->GetTransferDest();
    result["transfer_dest"] = dest;
    result["transfer_dest_name"] = (dest >= 0 && dest <= 4) ? transfer_names[dest] : "unknown";

    result["border_animation"] = sgb->GetBorderAnimation();
    result["border_empty"] = sgb->IsBorderEmpty();

    const u16* eff = sgb->GetEffectivePalettes();
    json palettes = json::array();
    for (int p = 0; p < 4; p++)
    {
        json pal = json::array();
        for (int c = 0; c < 4; c++)
        {
            char hex[8];
            snprintf(hex, sizeof(hex), "%04X", eff[p * 4 + c]);
            pal.push_back(hex);
        }
        palettes.push_back(pal);
    }
    result["effective_palettes"] = palettes;

    const u8* attrMap = sgb->GetAttributeMap();
    json attr = json::array();
    for (int y = 0; y < SGB_ATTR_MAP_HEIGHT; y++)
    {
        json row = json::array();
        for (int x = 0; x < SGB_ATTR_MAP_WIDTH; x++)
            row.push_back(attrMap[x + y * SGB_ATTR_MAP_WIDTH] & 3);
        attr.push_back(row);
    }
    result["attribute_map"] = attr;

    return result;
}

json DebugAdapter::LoadMedia(const std::string& file_path)
{
    json result;

    if (file_path.empty())
    {
        result["error"] = "File path is required";
        Log("[MCP] LoadMedia failed: File path is required");
        return result;
    }

    emu_load_rom_async(file_path.c_str(), false, Cartridge::CartridgeNotSupported, false);

    int timeout_ms = 180000;
    int elapsed_ms = 0;
    while (emu_is_rom_loading() && elapsed_ms < timeout_ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        elapsed_ms += 500;
    }

    if (emu_is_rom_loading())
    {
        result["error"] = "Loading timed out";
        Log("[MCP] LoadMedia timed out: %s", file_path.c_str());
        return result;
    }

    if (!emu_finish_rom_loading() || !m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "Failed to load media file";
        Log("[MCP] LoadMedia failed: %s", file_path.c_str());
        return result;
    }

    result["success"] = true;
    result["file_path"] = file_path;
    result["rom_name"] = m_core->GetCartridge()->GetFileName();
    result["is_cgb"] = m_core->GetCartridge()->IsCGB();

    return result;
}

json DebugAdapter::LoadSymbols(const std::string& file_path)
{
    json result;

    if (file_path.empty())
    {
        result["error"] = "File path is required";
        Log("[MCP] LoadSymbols failed: File path is required");
        return result;
    }

    gui_debug_load_symbols_file(file_path.c_str());

    result["success"] = true;
    result["file_path"] = file_path;

    return result;
}

json DebugAdapter::ListSaveStateSlots()
{
    json result;
    json slots = json::array();

    for (int i = 0; i < 5; i++)
    {
        json slot;
        slot["slot"] = i + 1;
        slot["selected"] = (config_emulator.save_slot == i);

        if (emu_savestates[i].rom_name[0] != 0)
        {
            slot["rom_name"] = emu_savestates[i].rom_name;
            slot["timestamp"] = emu_savestates[i].timestamp;
            slot["version"] = emu_savestates[i].version;
            slot["valid"] = (emu_savestates[i].version >= GB_SAVESTATE_MIN_VERSION && emu_savestates[i].version <= GB_SAVESTATE_VERSION);
            slot["has_screenshot"] = IsValidPointer(emu_savestates_screenshots[i].data);

            if (emu_savestates[i].emu_build[0] != 0)
                slot["emu_build"] = emu_savestates[i].emu_build;
        }
        else
        {
            slot["empty"] = true;
        }

        slots.push_back(slot);
    }

    result["slots"] = slots;
    result["current_slot"] = config_emulator.save_slot + 1;

    return result;
}

json DebugAdapter::SelectSaveStateSlot(int slot)
{
    json result;

    if (slot < 1 || slot > 5)
    {
        result["error"] = "Invalid slot number (must be 1-5)";
        Log("[MCP] SelectSaveStateSlot failed: Invalid slot %d", slot);
        return result;
    }

    config_emulator.save_slot = slot - 1;

    result["success"] = true;
    result["slot"] = slot;

    return result;
}

json DebugAdapter::SaveState()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        Log("[MCP] SaveState failed: No media loaded");
        return result;
    }

    int slot = config_emulator.save_slot + 1;
    emu_save_state_slot(slot);

    result["success"] = true;
    result["slot"] = slot;
    result["rom_name"] = m_core->GetCartridge()->GetFileName();

    return result;
}

json DebugAdapter::LoadState()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        Log("[MCP] LoadState failed: No media loaded");
        return result;
    }

    int slot = config_emulator.save_slot + 1;

    if (emu_savestates[config_emulator.save_slot].rom_name[0] == 0)
    {
        result["error"] = "Save state slot is empty";
        Log("[MCP] LoadState failed: Slot %d is empty", slot);
        return result;
    }

    emu_load_state_slot(slot);

    result["success"] = true;
    result["slot"] = slot;

    return result;
}

json DebugAdapter::SetFastForwardSpeed(int speed)
{
    json result;

    if (speed < 0 || speed > 4)
    {
        result["error"] = "Invalid speed (must be 0-4: 0=1.5x, 1=2x, 2=2.5x, 3=3x, 4=Unlimited)";
        Log("[MCP] SetFastForwardSpeed failed: Invalid speed %d", speed);
        return result;
    }

    config_emulator.ffwd_speed = speed;

    result["success"] = true;
    result["speed"] = speed;

    const char* speed_names[] = {"1.5x", "2x", "2.5x", "3x", "Unlimited"};
    result["speed_name"] = speed_names[speed];

    return result;
}

json DebugAdapter::ToggleFastForward(bool enabled)
{
    json result;

    config_emulator.ffwd = enabled;
    gui_action_ffwd();

    result["success"] = true;
    result["enabled"] = enabled;
    result["speed"] = config_emulator.ffwd_speed;

    return result;
}

json DebugAdapter::GetRewindStatus()
{
    json result;

    result["enabled"] = config_rewind.enabled;
    result["snapshot_count"] = rewind_get_snapshot_count();
    result["capacity"] = rewind_get_capacity();
    result["frames_per_snapshot"] = rewind_get_frames_per_snapshot();
    result["buffer_seconds"] = config_rewind.buffer_seconds;

    int fps = rewind_get_frames_per_snapshot();
    if (fps < 1)
        fps = 1;

    result["buffered_seconds"] = (double)(rewind_get_snapshot_count() * fps) / 60.0;

    return result;
}

json DebugAdapter::RewindSeek(int snapshot)
{
    bool paused = emu_is_paused() || emu_is_debug_idle();

    if (!paused)
        return {{"error", "Pause the emulator before seeking the rewind buffer"}};

    int count = rewind_get_snapshot_count();

    if (count == 0)
        return {{"error", "No rewind snapshots available"}};

    if (snapshot < 1 || snapshot > count)
        return {{"error", "Snapshot out of range (1-" + std::to_string(count) + ")"}};

    int age = count - snapshot;

    if (!gui_debug_rewind_seek(age))
        return {{"error", "Failed to load snapshot"}};

    Processor* cpu = m_core->GetProcessor();
    u16 pc = cpu->GetState()->PC->GetValue();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << pc;

    int fps = rewind_get_frames_per_snapshot();
    if (fps < 1)
        fps = 1;

    json result;
    result["success"] = true;
    result["snapshot"] = snapshot;
    result["total"] = count;
    result["age_seconds"] = (double)(age * fps) / 60.0;
    result["pc"] = ss.str();
    return result;
}

json DebugAdapter::ControllerButton(int player, const std::string& button, const std::string& action)
{
    json result;

    if (action != "press" && action != "release" && action != "press_and_release")
    {
        result["error"] = "Invalid action (must be: press, release, press_and_release)";
        return result;
    }

    if (player != 1)
    {
        result["error"] = "Game Boy only supports 1 player";
        return result;
    }

    std::string button_lower = button;
    std::transform(button_lower.begin(), button_lower.end(), button_lower.begin(), ::tolower);

    Gameboy_Keys key;
    bool key_found = true;

    if (button_lower == "up") key = Up_Key;
    else if (button_lower == "down") key = Down_Key;
    else if (button_lower == "left") key = Left_Key;
    else if (button_lower == "right") key = Right_Key;
    else if (button_lower == "a") key = A_Key;
    else if (button_lower == "b") key = B_Key;
    else if (button_lower == "start") key = Start_Key;
    else if (button_lower == "select") key = Select_Key;
    else
    {
        key_found = false;
        key = A_Key; // suppress uninitialized warning
    }

    if (!key_found)
    {
        result["error"] = "Invalid button name";
        return result;
    }

    if (action == "press")
    {
        emu_key_pressed(key);
    }
    else if (action == "release")
    {
        emu_key_released(key);
    }
    else if (action == "press_and_release")
    {
        emu_key_pressed(key);
        result["__delayed_release"] = true;
    }

    result["success"] = true;
    result["player"] = player;
    result["button"] = button;
    result["action"] = action;

    return result;
}

json DebugAdapter::ListSprites()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    Memory* memory = m_core->GetMemory();
    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_8x16 = (lcdc & 0x04) != 0;
    int sprite_height = sprites_8x16 ? 16 : 8;

    json sprites = json::array();

    for (int s = 0; s < 40; s++)
    {
        u16 oam_addr = 0xFE00 + (s * 4);
        u8 y = memory->Retrieve(oam_addr);
        u8 x = memory->Retrieve(oam_addr + 1);
        u8 tile = memory->Retrieve(oam_addr + 2);
        u8 attrs = memory->Retrieve(oam_addr + 3);

        json sprite_info;
        sprite_info["index"] = s;
        sprite_info["x"] = (int)x;
        sprite_info["y"] = (int)y;
        sprite_info["screen_x"] = (int)x - 8;
        sprite_info["screen_y"] = (int)y - 16;
        sprite_info["tile"] = (int)tile;
        sprite_info["oam_address"] = oam_addr;
        sprite_info["priority"] = (attrs & 0x80) != 0;
        sprite_info["y_flip"] = (attrs & 0x40) != 0;
        sprite_info["x_flip"] = (attrs & 0x20) != 0;
        sprite_info["dmg_palette"] = (attrs & 0x10) ? "OBP1" : "OBP0";
        sprite_info["cgb_vram_bank"] = (attrs & 0x08) >> 3;
        sprite_info["cgb_palette"] = attrs & 0x07;
        sprite_info["size"] = std::string("8x") + std::to_string(sprite_height);

        sprites.push_back(sprite_info);
    }

    result["sprites"] = sprites;

    return result;
}

json DebugAdapter::GetSpriteImage(int sprite_index)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (sprite_index < 0 || sprite_index > 39)
    {
        result["error"] = "Invalid sprite index (must be 0-39)";
        return result;
    }

    unsigned char* png_buffer = NULL;
    int png_size = emu_get_sprite_png(sprite_index, &png_buffer);

    if (png_size == 0 || !png_buffer)
    {
        result["error"] = "Failed to capture sprite";
        return result;
    }

    Memory* memory = m_core->GetMemory();
    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_8x16 = (lcdc & 0x04) != 0;
    int width = 8;
    int height = sprites_8x16 ? 16 : 8;

    std::string base64_png = base64_encode(png_buffer, png_size);
    free(png_buffer);

    result["__mcp_image"] = true;
    result["data"] = base64_png;
    result["mimeType"] = "image/png";
    result["width"] = width;
    result["height"] = height;
    result["sprite_index"] = sprite_index;

    return result;
}

// Disassembler operations

json DebugAdapter::RunToAddress(u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_runto_address(address);

    result["success"] = true;
    result["address"] = address;
    result["message"] = "Running to address";

    return result;
}

json DebugAdapter::AddDisassemblerBookmark(u16 address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_add_disassembler_bookmark(address, name.c_str());

    result["success"] = true;
    result["address"] = address;
    result["name"] = name.empty() ? "auto-generated" : name;

    return result;
}

json DebugAdapter::RemoveDisassemblerBookmark(u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_remove_disassembler_bookmark(address);

    result["success"] = true;
    result["address"] = address;

    return result;
}

json DebugAdapter::AddSymbol(u8 bank, u16 address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    char symbol[128];
    snprintf(symbol, sizeof(symbol), "%02X:%04X %s", bank, address, name.c_str());
    gui_debug_add_symbol(symbol);

    result["success"] = true;
    result["bank"] = bank;
    result["address"] = address;
    result["name"] = name;

    return result;
}

json DebugAdapter::RemoveSymbol(u8 bank, u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_remove_symbol(bank, address);

    result["success"] = true;
    result["bank"] = bank;
    result["address"] = address;

    return result;
}

// Memory editor operations

json DebugAdapter::SelectMemoryRange(int editor, int start_address, int end_address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    MemoryAreaInfo info = GetMemoryAreaInfo(editor);
    if (!IsValidPointer(info.data) || info.size == 0)
    {
        result["error"] = "Memory area unavailable";
        return result;
    }

    u32 display_base = GetMemoryAreaDisplayBase(editor);
    u32 start_offset = 0;
    u32 end_offset = 0;
    if (!NormalizeMemoryAreaAddress(info, display_base, (u32)start_address, &start_offset) ||
        !NormalizeMemoryAreaAddress(info, display_base, (u32)end_address, &end_offset))
    {
        result["error"] = "Selection range outside memory area";
        return result;
    }

    if (start_offset > end_offset)
        std::swap(start_offset, end_offset);

    u32 display_start = display_base + start_offset;
    u32 display_end = display_base + end_offset;

    if (!gui_debug_memory_select_range(editor, (int)display_start, (int)display_end))
    {
        result["error"] = "Unable to apply memory selection";
        return result;
    }

    int actual_start = -1;
    int actual_end = -1;
    gui_debug_memory_get_selection(editor, &actual_start, &actual_end);
    if (actual_start < 0 || actual_end < actual_start || (u32)actual_end >= info.size)
    {
        result["error"] = "Unable to read applied memory selection";
        return result;
    }

    u32 actual_display_start = display_base + (u32)actual_start;
    u32 actual_display_end = display_base + (u32)actual_end;

    std::ostringstream start_ss, end_ss;
    start_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << actual_display_start;
    end_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << actual_display_end;

    result["success"] = true;
    result["area"] = editor;
    result["start_address"] = start_ss.str();
    result["end_address"] = end_ss.str();

    return result;
}

json DebugAdapter::SetMemorySelectionValue(int editor, u8 value)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    gui_debug_memory_set_selection_value(editor, value);

    result["success"] = true;
    result["editor"] = editor;
    result["value"] = value;

    return result;
}

json DebugAdapter::AddMemoryBookmark(int editor, int address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    gui_debug_memory_add_bookmark(editor, address, name.c_str());

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;
    result["name"] = name.empty() ? "auto-generated" : name;

    return result;
}

json DebugAdapter::RemoveMemoryBookmark(int editor, int address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    gui_debug_memory_remove_bookmark(editor, address);

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;

    return result;
}

json DebugAdapter::AddMemoryWatch(int editor, int address, const std::string& notes, int size)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    MemoryAreaInfo info = GetMemoryAreaInfo(editor);
    u32 display_base = GetMemoryAreaDisplayBase(editor);
    if (!MemoryAreaContainsDisplayAddress(info, display_base, (u32)address))
    {
        result["error"] = "Watch address outside memory area";
        return result;
    }

    if (!gui_debug_memory_add_watch(editor, address, notes.c_str(), size))
    {
        result["error"] = "Unable to add memory watch";
        return result;
    }

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;
    result["notes"] = notes;
    result["size"] = size;

    return result;
}

json DebugAdapter::RemoveMemoryWatch(int editor, int address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number";
        return result;
    }

    gui_debug_memory_remove_watch(editor, address);

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;

    return result;
}

json DebugAdapter::ListDisassemblerBookmarks()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    void* bookmarks_ptr = NULL;
    int count = gui_debug_get_disassembler_bookmarks(&bookmarks_ptr);

    std::vector<DisassemblerBookmark>* bookmarks = (std::vector<DisassemblerBookmark>*)bookmarks_ptr;

    json bookmarks_array = json::array();

    if (bookmarks)
    {
        for (const DisassemblerBookmark& bookmark : *bookmarks)
        {
            json bookmark_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << bookmark.address;
            bookmark_obj["address"] = addr_ss.str();
            bookmark_obj["name"] = bookmark.name;

            bookmarks_array.push_back(bookmark_obj);
        }
    }

    result["bookmarks"] = bookmarks_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::ListSymbols()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    void* symbols_ptr = NULL;
    gui_debug_get_symbols(&symbols_ptr);

    DebugSymbol*** fixed_symbols = (DebugSymbol***)symbols_ptr;

    json symbols_array = json::array();

    if (fixed_symbols)
    {
        for (int bank = 0; bank < 0x100; bank++)
        {
            if (!fixed_symbols[bank])
                continue;

            for (int address = 0; address < 0x10000; address++)
            {
                if (fixed_symbols[bank][address])
                {
                    json symbol_obj;

                    std::ostringstream bank_ss, addr_ss;
                    bank_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << bank;
                    addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address;

                    symbol_obj["bank"] = bank_ss.str();
                    symbol_obj["address"] = addr_ss.str();
                    symbol_obj["name"] = fixed_symbols[bank][address]->text;

                    symbols_array.push_back(symbol_obj);
                }
            }
        }
    }

    result["symbols"] = symbols_array;
    result["count"] = symbols_array.size();

    return result;
}

json DebugAdapter::ListCallStack()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    Memory* memory = m_core->GetMemory();
    Processor* processor = m_core->GetProcessor();
    std::stack<Processor::GB_CallStackEntry> temp_stack = *processor->GetDisassemblerCallStack();

    void* symbols_ptr = NULL;
    gui_debug_get_symbols(&symbols_ptr);
    DebugSymbol*** fixed_symbols = (DebugSymbol***)symbols_ptr;

    json stack_array = json::array();

    while (!temp_stack.empty())
    {
        Processor::GB_CallStackEntry entry = temp_stack.top();
        temp_stack.pop();

        json entry_obj;

        std::ostringstream dest_ss, src_ss, back_ss;
        dest_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.dest;
        src_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.src;
        back_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.back;

        entry_obj["function"] = dest_ss.str();
        entry_obj["source"] = src_ss.str();
        entry_obj["return"] = back_ss.str();

        GB_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.dest);
        if (IsValidPointer(record) && record->name[0] != 0)
        {
            if (fixed_symbols && fixed_symbols[record->bank] && fixed_symbols[record->bank][entry.dest])
            {
                entry_obj["symbol"] = fixed_symbols[record->bank][entry.dest]->text;
            }
        }

        stack_array.push_back(entry_obj);
    }

    result["stack"] = stack_array;
    result["depth"] = stack_array.size();

    return result;
}

json DebugAdapter::ListMemoryBookmarks(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    void* bookmarks_ptr = NULL;
    int count = gui_debug_memory_get_bookmarks(area, &bookmarks_ptr);

    std::vector<MemEditor::Bookmark>* bookmarks = (std::vector<MemEditor::Bookmark>*)bookmarks_ptr;

    json bookmarks_array = json::array();

    if (bookmarks)
    {
        for (const MemEditor::Bookmark& bookmark : *bookmarks)
        {
            json bookmark_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << bookmark.address;
            bookmark_obj["address"] = addr_ss.str();
            bookmark_obj["name"] = bookmark.name;

            bookmarks_array.push_back(bookmark_obj);
        }
    }

    result["area"] = area;
    result["bookmarks"] = bookmarks_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::ListMemoryWatches(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    void* watches_ptr = NULL;
    int count = gui_debug_memory_get_watches(area, &watches_ptr);

    std::vector<MemEditor::Watch>* watches = (std::vector<MemEditor::Watch>*)watches_ptr;

    json watches_array = json::array();

    if (watches)
    {
        const char* size_names[] = {"8", "16", "24", "32"};
        const char* format_names[] = {"hex", "binary", "decimal_unsigned", "decimal_signed"};

        for (const MemEditor::Watch& watch : *watches)
        {
            json watch_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << watch.address;
            watch_obj["address"] = addr_ss.str();
            watch_obj["notes"] = watch.notes;

            int size_idx = (watch.size >= 0 && watch.size <= 3) ? watch.size : 0;
            int fmt_idx = (watch.format >= 0 && watch.format <= 3) ? watch.format : 0;
            watch_obj["size"] = size_names[size_idx];
            watch_obj["format"] = format_names[fmt_idx];

            watches_array.push_back(watch_obj);
        }
    }

    result["area"] = area;
    result["watches"] = watches_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::GetMemorySelection(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    int start = -1;
    int end = -1;
    gui_debug_memory_get_selection(area, &start, &end);

    result["area"] = area;

    if (start >= 0 && end >= 0 && start <= end)
    {
        std::ostringstream start_ss, end_ss;
        start_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << start;
        end_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << end;

        result["start"] = start_ss.str();
        result["end"] = end_ss.str();
        result["size"] = end - start + 1;
    }
    else
    {
        result["start"] = NULL;
        result["end"] = NULL;
        result["size"] = 0;
        result["note"] = "No selection";
    }

    return result;
}

json DebugAdapter::MemorySearchCapture(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    gui_debug_memory_search_capture(area);

    result["success"] = true;
    result["area"] = area;
    result["message"] = "Memory snapshot captured";

    return result;
}

json DebugAdapter::MemorySearch(int area, const std::string& op, const std::string& compare_type, int compare_value, const std::string& data_type)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    int op_index = 0;
    if (op == "<") op_index = 0;
    else if (op == ">") op_index = 1;
    else if (op == "==") op_index = 2;
    else if (op == "!=") op_index = 3;
    else if (op == "<=") op_index = 4;
    else if (op == ">=") op_index = 5;
    else
    {
        result["error"] = "Invalid operator";
        return result;
    }

    int compare_type_index = 0;
    if (compare_type == "previous") compare_type_index = 0;
    else if (compare_type == "value") compare_type_index = 1;
    else if (compare_type == "address") compare_type_index = 2;
    else
    {
        result["error"] = "Invalid compare_type";
        return result;
    }

    int data_type_index = 0;
    if (data_type == "hex") data_type_index = 0;
    else if (data_type == "signed") data_type_index = 1;
    else if (data_type == "unsigned") data_type_index = 2;
    else
    {
        result["error"] = "Invalid data_type";
        return result;
    }

    void* results_ptr = NULL;
    int count = gui_debug_memory_search(area, op_index, compare_type_index, compare_value, data_type_index, &results_ptr);

    result["area"] = area;
    result["count"] = count;
    result["results"] = json::array();

    if (count > 0 && results_ptr != NULL)
    {
        std::vector<MemEditor::Search>* results = (std::vector<MemEditor::Search>*)results_ptr;

        int max_results = (count > 1000) ? 1000 : count;

        for (int i = 0; i < max_results; i++)
        {
            MemEditor::Search& search = (*results)[i];
            json item;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << search.address;

            item["address"] = addr_ss.str();
            item["value"] = search.value;
            item["previous"] = search.prev_value;

            result["results"].push_back(item);
        }

        if (count > 1000)
        {
            result["note"] = "Results limited to first 1000 matches";
            result["total_matches"] = count;
        }
    }

    return result;
}

json DebugAdapter::MemoryFindBytes(int area, const std::string& hex_bytes)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsLoadedROM())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    if (hex_bytes.empty())
    {
        result["error"] = "Empty hex byte string";
        return result;
    }

    int addresses[100];
    int count = gui_debug_memory_find_bytes(area, hex_bytes.c_str(), addresses, 100);

    result["area"] = area;
    result["count"] = count;
    result["results"] = json::array();

    int max_results = (count > 100) ? 100 : count;

    for (int i = 0; i < max_results; i++)
    {
        json item;
        std::ostringstream addr_ss;
        addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addresses[i];
        item["address"] = addr_ss.str();
        result["results"].push_back(item);
    }

    if (count > 100)
    {
        result["note"] = "Results limited to first 100 matches";
        result["total_matches"] = count;
    }

    return result;
}

json DebugAdapter::GetTraceLog(int start, int count)
{
    json result;

    TraceLogger* tl = m_core->GetTraceLogger();
    if (!tl)
    {
        result["error"] = "Trace logger not available";
        return result;
    }

    u32 total = tl->GetCount();

    if (count < 1) count = 100;
    if (count > 1000) count = 1000;

    u32 actual_start;
    if (start < 0)
        actual_start = (total > (u32)count) ? (total - (u32)count) : 0;
    else
        actual_start = (u32)start;

    if (actual_start >= total)
    {
        result["total_entries"] = total;
        result["start"] = actual_start;
        result["count"] = 0;
        result["lines"] = json::array();
        return result;
    }

    u32 actual_count = (u32)count;
    if (actual_start + actual_count > total)
        actual_count = total - actual_start;

    Memory* memory = m_core->GetMemory();

    json lines = json::array();

    for (u32 i = 0; i < actual_count; i++)
    {
        const GB_Trace_Entry& entry = tl->GetEntry(actual_start + i);
        char buf[256];

        switch (entry.type)
        {
            case TRACE_CPU:
            {
                GB_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.cpu.pc, entry.cpu.bank);
                char instr[64] = "???";
                char bytes[25] = "";
                if (IsValidPointer(record))
                {
                    strncpy(instr, record->name, sizeof(instr) - 1);
                    instr[sizeof(instr) - 1] = '\0';
                    char* p = instr;
                    while (*p)
                    {
                        if (*p == '{')
                        {
                            char* end = strchr(p, '}');
                            if (end)
                                memmove(p, end + 1, strlen(end + 1) + 1);
                            else
                                break;
                        }
                        else
                            p++;
                    }
                    strncpy(bytes, record->bytes, sizeof(bytes) - 1);
                    bytes[sizeof(bytes) - 1] = '\0';
                }
                u8 a = (entry.cpu.af >> 8) & 0xFF;
                u8 f = entry.cpu.af & 0xFF;
                snprintf(buf, sizeof(buf), "%02X:%04X  A:%02X  BC:%04X  DE:%04X  HL:%04X  SP:%04X  %c%c%c%c  %-24s %s",
                         entry.cpu.bank, entry.cpu.pc, a,
                         entry.cpu.bc, entry.cpu.de, entry.cpu.hl, entry.cpu.sp,
                         (f & FLAG_ZERO) ? 'Z' : 'z',
                         (f & FLAG_SUB) ? 'N' : 'n',
                         (f & FLAG_HALF) ? 'H' : 'h',
                         (f & FLAG_CARRY) ? 'C' : 'c',
                         instr, bytes);
                break;
            }
            case TRACE_CPU_IRQ:
            {
                static const char* k_irq_names[] = {"???", "VBlank", "LCDSTAT", "Timer", "Serial", "Joypad"};
                const char* irq_name = (entry.irq.type >= 1 && entry.irq.type <= 5) ? k_irq_names[entry.irq.type] : "???";
                snprintf(buf, sizeof(buf), "  [CPU]  %-8s  PC:$%04X  Vector:$%04X",
                         irq_name, entry.irq.pc, entry.irq.vector);
                break;
            }
            case TRACE_LCD_WRITE:
            {
                static const char* k_lcd_regs[] = {"LCDC", "STAT", "SCY", "SCX", "LY", "LYC", "DMA", "BGP", "OBP0", "OBP1", "WY", "WX"};
                const char* reg_name = (entry.lcd_write.reg < 12) ? k_lcd_regs[entry.lcd_write.reg] : "???";
                snprintf(buf, sizeof(buf), "  [LCD]  %-5s    Value:$%02X",
                         reg_name, entry.lcd_write.value);
                break;
            }
            case TRACE_LCD_STATUS:
            {
                static const char* k_lcd_events[] = {"VBLANK", "STAT", "LYC", "MODE", "HDMA"};
                const char* event_name = (entry.lcd_status.event < 5) ? k_lcd_events[entry.lcd_status.event] : "???";
                switch (entry.lcd_status.event)
                {
                    case GB_LCD_EVENT_VBLANK:
                    case GB_LCD_EVENT_LYC_MATCH:
                    case GB_LCD_EVENT_HDMA:
                        snprintf(buf, sizeof(buf), "  [LCD]  %-9s Line:%d",
                                 event_name, entry.lcd_status.line);
                        break;
                    case GB_LCD_EVENT_STAT_IRQ:
                    case GB_LCD_EVENT_MODE_CHANGE:
                        snprintf(buf, sizeof(buf), "  [LCD]  %-9s Line:%d  Mode:%d",
                                 event_name, entry.lcd_status.line, entry.lcd_status.value);
                        break;
                    default:
                        snprintf(buf, sizeof(buf), "  [LCD]  %-9s Line:%d",
                                 event_name, entry.lcd_status.line);
                        break;
                }
                break;
            }
            case TRACE_APU_WRITE:
                snprintf(buf, sizeof(buf), "  [APU]  WRITE    Addr:$%04X  Value:$%02X",
                         entry.apu_write.address, entry.apu_write.value);
                break;
            case TRACE_IO_WRITE:
                snprintf(buf, sizeof(buf), "  [IO]   %s     Addr:$%04X  Value:$%02X",
                         entry.io_write.is_write ? "OUT" : "IN ",
                         entry.io_write.address, entry.io_write.value);
                break;
            case TRACE_BANK_SWITCH:
                snprintf(buf, sizeof(buf), "  [MAP]  BANK     Addr:$%04X  Value:$%02X",
                         entry.bank_switch.address, entry.bank_switch.value);
                break;
            default:
                snprintf(buf, sizeof(buf), "  [???]");
                break;
        }

        lines.push_back(buf);
    }

    result["total_entries"] = total;
    result["start"] = actual_start;
    result["count"] = actual_count;
    result["lines"] = lines;
    return result;
}

json DebugAdapter::SetTraceLog(bool enabled, u32 flags)
{
    json result;

    TraceLogger* tl = m_core->GetTraceLogger();
    if (!tl)
    {
        result["error"] = "Trace logger not available";
        return result;
    }

    if (enabled)
    {
        if (flags == 0)
            flags = TRACE_FLAG_CPU;
        tl->SetEnabledFlags(flags);

        result["status"] = "started";
        result["enabled_flags"] = flags;

        json enabled_list = json::array();
        if (flags & TRACE_FLAG_CPU) enabled_list.push_back("cpu");
        if (flags & TRACE_FLAG_CPU_IRQ) enabled_list.push_back("cpu_irq");
        if (flags & TRACE_FLAG_LCD_WRITE) enabled_list.push_back("lcd_write");
        if (flags & TRACE_FLAG_LCD_STATUS) enabled_list.push_back("lcd_status");
        if (flags & TRACE_FLAG_APU_WRITE) enabled_list.push_back("apu_write");
        if (flags & TRACE_FLAG_IO_WRITE) enabled_list.push_back("io_write");
        if (flags & TRACE_FLAG_BANK_SWITCH) enabled_list.push_back("bank_switch");
        result["enabled"] = enabled_list;
    }
    else
    {
        tl->SetEnabledFlags(0);
        result["status"] = "stopped";
    }

    result["total_entries"] = tl->GetCount();
    return result;
}
