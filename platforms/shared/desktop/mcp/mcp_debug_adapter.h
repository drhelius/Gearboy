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

#ifndef MCP_DEBUG_ADAPTER_H
#define MCP_DEBUG_ADAPTER_H

#include <vector>
#include <string>
#include "json.hpp"
#include "gearboy.h"
#include "../gui_debug_memory.h"

using json = nlohmann::json;

struct MemoryAreaInfo
{
    int id;
    std::string name;
    u32 size;
    u8* data;
};

struct RegistersSnapshot
{
    u16 AF, BC, DE, HL, SP, PC;
    bool IME;
    bool Halt;
    bool DoubleSpeed;
};

struct BreakpointInfo
{
    bool enabled;
    int type;
    u16 address1;
    u16 address2;
    bool read;
    bool write;
    bool execute;
    bool range;
    std::string type_name;
};

struct DisasmLine
{
    u32 address;
    u8 bank;
    std::string name;
    std::string bytes;
    std::string segment;
    int size;
    bool jump;
    u16 jump_address;
    u8 jump_bank;
    bool has_operand_address;
    u16 operand_address;
    bool subroutine;
    int irq;
};

class DebugAdapter
{
public:
    DebugAdapter(GearboyCore* core)
    {
        m_core = core;
    }

    // Execution control
    void Pause();
    void Resume();
    void StepInto();
    void StepOver();
    void StepOut();
    void StepFrame();
    void Reset();
    json GetDebugStatus();
    json RunToAddress(u16 address);

    // Breakpoints
    void SetBreakpoint(u16 address, int type, bool read, bool write, bool execute);
    void SetBreakpointRange(u16 start_address, u16 end_address, int type, bool read, bool write, bool execute);
    void ClearBreakpointByAddress(u16 address, int type, u16 end_address = 0);
    std::vector<BreakpointInfo> ListBreakpoints();

    // Registers
    RegistersSnapshot GetRegisters();
    void SetRegister(const std::string& name, u32 value);

    // Memory areas (matching debugger memory editor)
    std::vector<MemoryAreaInfo> ListMemoryAreas();
    std::vector<u8> ReadMemoryArea(int area, u32 offset, size_t size);
    void WriteMemoryArea(int area, u32 offset, const std::vector<u8>& data);

    // Disassembly (using existing disassembler records)
    std::vector<DisasmLine> GetDisassembly(u16 start_address, u16 end_address, int bank = -1, bool resolve_symbols = false);

    // Chip status info
    json GetCPUStatus();
    json GetLCDRegisters();
    json GetLCDStatus();
    json GetAPUStatus();
    json GetScreenshot();
    json ListSprites();
    json GetSpriteImage(int sprite_index);

    // Media and state management
    json GetMediaInfo();
    json LoadMedia(const std::string& file_path);
    json ListSaveStateSlots();
    json SelectSaveStateSlot(int slot);
    json SaveState();
    json LoadState();
    json SetFastForwardSpeed(int speed);
    json ToggleFastForward(bool enabled);

    // Controller input
    json ControllerButton(int player, const std::string& button, const std::string& action);

    // Disassembler operations
    json AddDisassemblerBookmark(u16 address, const std::string& name);
    json RemoveDisassemblerBookmark(u16 address);
    json ListDisassemblerBookmarks();
    json AddSymbol(u8 bank, u16 address, const std::string& name);
    json RemoveSymbol(u8 bank, u16 address);
    json LoadSymbols(const std::string& file_path);
    json ListSymbols();
    json ListCallStack();

    // Memory area operations
    json SelectMemoryRange(int area, int start_address, int end_address);
    json SetMemorySelectionValue(int area, u8 value);
    json GetMemorySelection(int area);
    json AddMemoryBookmark(int area, int address, const std::string& name);
    json RemoveMemoryBookmark(int area, int address);
    json ListMemoryBookmarks(int area);
    json AddMemoryWatch(int area, int address, const std::string& notes, int size);
    json RemoveMemoryWatch(int area, int address);
    json ListMemoryWatches(int area);
    json MemorySearchCapture(int area);
    json MemorySearch(int area, const std::string& op, const std::string& compare_type, int compare_value, const std::string& data_type);
    json MemoryFindBytes(int area, const std::string& hex_bytes);
    json GetTraceLog(int start, int count);

    // Core access
    GearboyCore* GetCore() { return m_core; }

private:
    GearboyCore* m_core;

    const char* GetBreakpointTypeName(int type);
    MemoryAreaInfo GetMemoryAreaInfo(int area);
};

#endif /* MCP_DEBUG_ADAPTER_H */
