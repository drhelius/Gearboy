---
name: gearboy-debugging
description: >-
  Debug and trace Game Boy / Game Boy Color games using the Gearboy emulator MCP
  server. Provides workflows for SM83 CPU debugging, breakpoint management,
  hardware inspection, disassembly analysis, and execution tracing. Use when the
  user wants to debug a Game Boy game, trace code execution, inspect CPU
  registers or hardware state, set breakpoints, analyze interrupts, step through
  SM83 instructions, reverse engineer game code, examine LCD or APU registers,
  view the call stack, or diagnose rendering, audio, or timing issues. Also use
  when the user mentions Game Boy development, GB/GBC homebrew testing, or SM83
  debugging with Gearboy.
compatibility: >-
  Requires the Gearboy MCP server. Before installing or configuring, call
  debug_get_status to check if the server is already connected. If it responds,
  the server is ready — skip setup entirely.
metadata:
  author: drhelius
  version: "1.0"
---

# Game Boy / Game Boy Color Debugging with Gearboy

## Overview

Debug Game Boy and Game Boy Color games using the Gearboy emulator as an MCP server. Control execution (pause, step, breakpoints), inspect the SM83 CPU and hardware (LCD, APU, sprites), read/write memory, disassemble code, trace instructions, and capture screenshots — all through MCP tool calls.

## MCP Server Prerequisite

**IMPORTANT — Check before installing:** Before attempting any installation or configuration, you MUST first verify if the Gearboy MCP server is already connected in your current session. Call `debug_get_status` — if it returns a valid response, the server is active and ready.

Only if the tool is not available or the call fails, you need to help install and configure the Gearboy MCP server:

### Installing Gearboy

Run the bundled install script (macOS/Linux):

```bash
bash scripts/install.sh
```

This installs Gearboy via Homebrew on macOS or downloads the latest release on Linux. It prints the binary path on completion. You can also set `GEARBOY_INSTALL_DIR` to control where the binary goes (default: `~/.local/bin`).

Alternatively, download from [GitHub Releases](https://github.com/drhelius/Gearboy/releases/latest) or install with `brew install --cask drhelius/geardome/gearboy` on macOS.

### Connecting as MCP Server

Configure your AI client to run Gearboy as an MCP server via STDIO transport. Example for Claude Desktop (`~/Library/Application Support/Claude/claude_desktop_config.json`):
```json
{
  "mcpServers": {
    "gearboy": {
      "command": "/path/to/gearboy",
      "args": ["--mcp-stdio"]
    }
  }
}
```
Replace `/path/to/gearboy` with the actual binary path from the install script. Add `--headless` before `--mcp-stdio` on headless machines.

---

## Debugging Workflow

### 1. Load and Orient

```
load_media → get_media_info → get_cpu_status → get_screenshot
```

Start every session by loading the ROM, confirming it loaded correctly (MBC type, ROM/RAM size, CGB/SGB flags), then checking CPU state and taking a screenshot to understand the current game state. If a `.sym` or `.noi` file exists alongside the ROM, symbols are loaded automatically.

Load additional symbols with `load_symbols` or add individual labels with `add_symbol`. Gearboy supports RGBDS, GBDK-2020, WLA-DX, no$gmb, SDCC/NoICE (.noi), EQU, and generic symbol formats.

### 2. Pause and Inspect

Always call `debug_pause` before inspecting state. While paused:

- **CPU state**: `get_cpu_status` — registers A, F, B, C, D, E, H, L, SP, PC, flags Z/N/H/C, IME, halt state, CGB double speed
- **Disassembly**: `get_disassembly` with a start/end address range
- **Call stack**: `get_call_stack` — current subroutine hierarchy
- **Memory**: `read_memory` with area name (ROM0, ROM1, VRAM, RAM, WRAM0, WRAM1, WRAM, OAM, IO, HIRAM) and address/length

### 3. Set Breakpoints

Use breakpoints to stop execution at points of interest:

| Breakpoint Type | Tool | Use Case |
|---|---|---|
| Execution | `set_breakpoint` (type: exec) | Stop when PC reaches address |
| Read | `set_breakpoint` (type: read) | Stop when memory address is read |
| Write | `set_breakpoint` (type: write) | Stop when memory address is written |
| Range | `set_breakpoint_range` | Cover an address range (exec/read/write) |
| IRQ | `toggle_irq_breakpoints` | Break on VBlank, LCD STAT, Timer, Serial, or Joypad interrupts |

Breakpoints support 3 memory area types: `rom_ram`, `vram`, and `io`.

**Important**: Read/write breakpoints stop with PC at the instruction *after* the memory access.

Manage breakpoints with `list_breakpoints` and `remove_breakpoint`.

### 4. Step Through Code

After hitting a breakpoint or pausing:

| Action | Tool | Behavior |
|---|---|---|
| Step Into | `debug_step_into` | Execute one SM83 instruction, enter subroutines |
| Step Over | `debug_step_over` | Execute one instruction, skip CALL instructions |
| Step Out | `debug_step_out` | Run until RET/RETI returns from current subroutine |
| Step Frame | `debug_step_frame` | Execute until next VBlank |
| Run To | `debug_run_to_cursor` | Continue until PC reaches target address |
| Continue | `debug_continue` | Resume normal execution |

After each step, call `get_cpu_status` and `get_disassembly` to see where you are.

### 5. Trace Execution

The trace logger records CPU instructions interleaved with hardware events (LCD, APU, I/O, bank switching). Start the trace logger from the emulator's debugger window, then:

1. `set_trace_log` with `enabled: true` to start recording (optionally filter event types)
2. Let the game run or step through code
3. `set_trace_log` with `enabled: false` to stop (entries are preserved)
4. `get_trace_log` to read recorded entries

Tracing is essential for understanding timing-sensitive code, interrupt handlers, and hardware interaction sequences.

---

## Hardware Inspection

### LCD Controller

- `get_lcd_registers` — all LCD registers: LCDC, STAT, SCY, SCX, LY, LYC, DMA, BGP, OBP0, OBP1, WY, WX with decoded bit fields. CGB registers: KEY1, VBK, HDMA, BCPS, BCPD, OCPS, OCPD, SVBK
- `get_lcd_status` — current LCD mode (0-3), screen enabled, LY, LYC match, CGB info

### APU (Audio)

- `get_apu_status` — all 4 audio channels: Square 1 (with sweep), Square 2, Wave, Noise: volume, frequency, envelope, duty cycle, wave RAM, panning, master volume

### Sprites (OAM)

- `list_sprites` — all 40 OAM sprites: position, tile index, attributes (priority, X/Y flip, palette, CGB bank)
- `get_sprite_image` — individual sprite rendered as PNG

### Screen Capture

- `get_screenshot` — current rendered frame as PNG

Use screenshots after stepping or continuing to see the visual impact of changes.

---

## Memory Areas

Use `list_memory_areas` to get the full list:

| Area | Description |
|---|---|
| ROM0 | ROM bank 0 (fixed, $0000-$3FFF) |
| ROM1 | Switchable ROM bank ($4000-$7FFF) |
| VRAM | Video RAM — tiles and tile maps ($8000-$9FFF) |
| RAM | Cartridge external RAM ($A000-$BFFF) |
| WRAM0 | Work RAM bank 0 ($C000-$CFFF) |
| WRAM1 | Switchable Work RAM bank 1-7 on CGB ($D000-$DFFF) |
| WRAM | Full Work RAM ($C000-$DFFF) |
| OAM | Object Attribute Memory — sprite table ($FE00-$FE9F) |
| IO | I/O registers ($FF00-$FF7F) |
| HIRAM | High RAM ($FF80-$FFFE) |

WRAM and HIRAM are the most common locations for game variables (lives, health, score, position).

---

## Common Debugging Scenarios

### Finding an Interrupt Handler

1. `toggle_irq_breakpoints` to enable breaking on the target IRQ (VBlank, LCD STAT, Timer, Serial, Joypad)
2. `debug_continue` to run until the IRQ fires
3. `get_cpu_status` + `get_disassembly` to see the handler code
4. `get_call_stack` to see how deep you are
5. `add_symbol` to label the handler address and any subroutines it calls

The Game Boy interrupt vectors are at fixed addresses: VBlank=$0040, LCD STAT=$0048, Timer=$0050, Serial=$0058, Joypad=$0060.

### Diagnosing Graphics Corruption

1. `debug_pause` → `get_lcd_registers` — check LCDC (enable bits), SCX/SCY (scroll), WX/WY (window)
2. `get_lcd_status` — verify LCD mode, LY position
3. `read_memory` on VRAM to inspect tile data and tile maps
4. `list_sprites` — check OAM for incorrect positions, tiles, or attributes
5. Set read/write breakpoints on VRAM addresses to catch corruption source
6. `get_screenshot` to see the current visual state

### Analyzing a Subroutine

1. `set_breakpoint` at the subroutine entry point
2. `debug_continue` → when hit, `get_cpu_status`
3. Step through with `debug_step_into` / `debug_step_over`
4. After each step: check registers, read relevant memory
5. `add_symbol` for the routine and any called subroutines
6. `add_disassembler_bookmark` to mark interesting locations

### Tracking a Variable

1. `add_memory_watch` on the variable's address — watches are visible in the emulator GUI
2. Set a write breakpoint with `set_breakpoint` (type: write) on that address
3. When hit, `get_disassembly` reveals what code is modifying it
4. `get_call_stack` shows the call chain leading to the write

### Timing Analysis

1. `toggle_irq_breakpoints` to break on Timer or VBlank
2. Start the trace logger from the debugger window
3. `get_trace_log` to see the interleaved CPU + hardware events
4. `get_lcd_registers` to check timer and LCD timing configuration
5. Correlate interrupt fires with code execution in the trace

### Investigating CGB Features

For Game Boy Color games:

1. `get_cpu_status` — check if CGB double speed mode is active
2. `get_lcd_registers` — inspect CGB-specific registers (KEY1, VBK, HDMA, BCPS/BCPD, OCPS/OCPD, SVBK)
3. `read_memory` on VRAM with bank selection to inspect both VRAM banks
4. `list_sprites` — check CGB-specific attributes (VRAM bank, CGB palette)

---

## Organizing Your Debug Session

- **Symbols**: Use `add_symbol` liberally to label addresses you've identified — makes disassembly readable
- **Bookmarks**: Use `add_disassembler_bookmark` for code locations and `add_memory_bookmark` for data regions
- **Watches**: Use `add_memory_watch` for variables you're tracking across steps
- **Save states**: Use `save_state` / `load_state` to snapshot and restore emulator state at interesting points
- **Screenshots**: Capture visual state with `get_screenshot` after significant changes
