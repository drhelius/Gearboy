# Gearboy Agent Skills

[Agent Skills](https://agentskills.io/) for the Gearboy Game Boy / Game Boy Color emulator MCP server. These skills teach AI agents how to effectively use Gearboy's MCP tools for debugging and ROM hacking tasks.

## Prerequisites

All skills require the **Gearboy emulator** running as an MCP server. The emulator must be configured in your AI client (VS Code, Claude Desktop, Claude Code, etc.) so the agent can access the MCP tools.

See [MCP_README.md](../MCP_README.md) for complete setup instructions (STDIO, HTTP, VS Code, Claude Desktop, Claude Code).

## Installation

The recommended way to install the skills is using the [```skills```](https://skills.sh/docs) CLI, which requires no prior installation:

```bash
npx skills add drhelius/gearboy
```

Or install a specific skill:

```bash
npx skills add drhelius/gearboy --skill gearboy-debugging
npx skills add drhelius/gearboy --skill gearboy-romhacking
```

This downloads and configures the skills for use with your AI agent. See the [skills CLI reference](https://skills.sh/docs/cli) for more details.

## Available Skills

### gearboy-debugging

**Purpose**: Game development, debugging, and tracing of Game Boy / Game Boy Color games.

**What it covers**:
- Loading ROMs and debug symbols (RGBDS, GBDK-2020, WLA-DX, no$gmb, SDCC/NoICE, EQU formats)
- SM83 CPU register and flag inspection
- Setting execution, read, write, range, and IRQ breakpoints
- Stepping through code (into, over, out, frame, run-to)
- Execution tracing with interleaved hardware events (LCD, APU, I/O, bank switching)
- Hardware inspection: LCD controller, APU audio channels, OAM sprites
- Screenshot capture
- Call stack analysis
- CGB-specific debugging (double speed, VRAM banks, CGB palettes)
- Organizing debug sessions with symbols, bookmarks, and watches

**Key MCP tools used**: `debug_pause`, `debug_step_into`, `debug_step_over`, `debug_step_out`, `set_breakpoint`, `toggle_irq_breakpoints`, `get_cpu_status`, `get_disassembly`, `get_call_stack`, `get_trace_log`, `get_lcd_registers`, `get_apu_status`, `list_sprites`, `add_symbol`, `get_screenshot`

**Example prompts**:
- "Find the VBlank interrupt handler and analyze what it does"
- "Set a breakpoint at $0150 and step through the code"
- "The game has corrupted graphics — diagnose the issue"
- "Trace the sprite update routine and explain the algorithm"

### gearboy-romhacking

**Purpose**: Creating modifications, cheats, translations, and ROM hacks for Game Boy / Game Boy Color games.

**What it covers**:
- Memory search workflows (capture → change → compare cycle)
- Finding game variables (lives, health, score, position)
- Creating cheats (infinite lives, score modification, etc.)
- Game Genie and GameShark code discovery
- Text and string discovery for translations
- Sprite and tile data location
- Data table and structure reverse engineering
- Save state management for safe experimentation
- Fast forwarding to reach specific game states

**Key MCP tools used**: `memory_search_capture`, `memory_search`, `memory_find_bytes`, `read_memory`, `write_memory`, `set_breakpoint` (write type), `add_memory_watch`, `add_memory_bookmark`, `save_state`, `load_state`, `toggle_fast_forward`, `get_screenshot`, `list_sprites`, `get_sprite_image`, `controller_button`

**Example prompts**:
- "Find the lives counter and give me infinite lives"
- "Search for the score variable in memory"
- "Find all text strings in the ROM for translation"
- "Locate the sprite data for the player character"
