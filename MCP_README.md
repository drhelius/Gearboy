# Gearboy MCP Server

A [Model Context Protocol](https://modelcontextprotocol.io/introduction) server for the Gearboy emulator, enabling AI-assisted debugging and development of Nintendo Game Boy / Game Boy Color games.

This server provides tools for game development, rom hacking, reverse engineering, and debugging through standardized MCP protocols compatible with AI agents like Claude Code, Codex and others.

## Downloads

These MCP Bundles (`.mcpb`) package Gearboy with its MCP server configuration for MCP clients that install bundles, and are the packages published to the [MCP Registry](https://registry.modelcontextprotocol.io/). Claude Code and Codex use a regular Gearboy build instead, see [Quick Start](#quick-start).

<table>
  <thead>
    <tr>
      <th>Platform</th>
      <th>Architecture</th>
      <th>Download Link</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="2"><strong>Windows</strong></td>
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-windows-x64.mcpb">Gearboy-3.8.19-mcpb-windows-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-windows-arm64.mcpb">Gearboy-3.8.19-mcpb-windows-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="2"><strong>macOS</strong></td>
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-macos-x64.mcpb">Gearboy-3.8.19-mcpb-macos-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-macos-arm64.mcpb">Gearboy-3.8.19-mcpb-macos-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="2"><strong>Linux</strong></td>
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-linux-x64.mcpb">Gearboy-3.8.19-mcpb-linux-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.8.19/Gearboy-3.8.19-mcpb-linux-arm64.mcpb">Gearboy-3.8.19-mcpb-linux-arm64.mcpb</a></td>
    </tr>
  </tbody>
</table>

## Features

- **Full Debugger Access**: CPU registers, memory inspection, breakpoints, and execution control
- **Multiple Memory Areas**: Access ROM banks, VRAM, WRAM, OAM, IO registers, HIRAM, and SGB-specific areas (border tiles, tilemap, palettes, attribute files)
- **Disassembly**: View disassembled SM83 code around PC or any address
- **Hardware Inspection**: SM83 CPU, LCD controller, APU audio channels
- **Sprite Viewer**: List and inspect all 40 OAM sprites with images
- **Symbol Support**: Add, remove, list, and look up debug symbols
- **Input State**: Inspect effective pressed buttons and pending tap releases
- **Bookmarks**: Memory and disassembler bookmarks for navigation
- **Call Stack**: View function call hierarchy
- **Trace Logger**: CPU instruction trace with interleaved LCD, input, timer, APU, serial, and mapper events
- **Profiler**: Per-function call counts, calls per frame, and inclusive/exclusive cycle statistics
- **Rewind**: Time-travel debugging with snapshot status and seek tools
- **Screenshot Capture**: Get current frame as PNG image
- **Video Recording**: Record emulated video and audio to AVI files on disk
- **GUI Integration**: MCP server runs alongside the emulator GUI, sharing the same state

## Transport Modes

The Gearboy MCP server supports two transport modes:

### STDIO Transport (Recommended)

The default mode uses standard input/output for communication. The emulator is launched by the AI client and communicates through stdin/stdout pipes.

### HTTP Transport

The HTTP transport mode runs the emulator with an embedded web server on `127.0.0.1:7777/mcp` by default. The emulator stays running independently while the AI client connects via HTTP. Each request's `Host` and browser `Origin` must match the address on which its connection reached the server. Loopback mode can run without authentication; wildcard and other non-loopback bind addresses require `GEARBOY_MCP_HTTP_TOKEN`, and the server refuses to start without it.

### Headless Mode

Add `--headless` to run without a GUI window. This is useful for servers, CLI agents, or any machine without a display. All MCP tools work identically in headless mode. Headless mode requires `--mcp-stdio`, `--mcp-http`, or `--link-cable-join N`; link-only mode does not expose MCP tools.

A save state can be loaded at launch too, with `--load-state 1` for slot 1 or `--load-state FILE` for a state file; run `gearboy --help` for the full list.

### Pause When Inactive

While the MCP server is running, the emulator keeps running when its window loses focus, even with **Pause When Inactive** enabled in the **Emulator** menu. The setting keeps its value and applies again once the server stops.

### Concurrent Clients

The HTTP server accepts repeated valid MCP initialization requests. All connected clients control the same Gearboy instance. Individual HTTP requests are serialized, but multi-request debugging workflows are not atomic. Concurrent agents can interfere with each other through pauses, resets, breakpoints, memory writes, media loads, and save states.

For independent agent tasks, run one Gearboy instance per agent on a unique HTTP port. Use `--headless` and give each instance its own portable application directory so its configuration and runtime files are isolated:

```bash
./gearboy --mcp-http --headless --portable --mcp-http-port 7778
```

The `--portable` option stores configuration and user data beside the application. Alternatively, create an empty `portable.ini` beside the executable in each application directory. On macOS, place it next to each `.app` bundle.

## MCP Tool Router

By default, Gearboy exposes every MCP tool directly. This avoids nested tool discovery in clients that already defer MCP schemas, including Claude Code.

Add `--mcp-router` to expose a compact set of high-frequency tools directly and route advanced debugger tools through lightweight discovery tools. This reduces MCP context while preserving access to the full debugger surface.

Direct tools in routed mode: `load_media`, `get_media_info`, `debug_pause`, `debug_continue`, `debug_step_into`, `get_cpu_status`, `read_memory`, `write_memory`, `get_disassembly`, `set_breakpoint`, `get_screenshot`, and `controller_button`.

Router tools:

- `list_tool_categories` lists routed tool categories with descriptions and tool counts.
- `get_category_tools` lists routed tools in a category with compact descriptions.
- `search_tools` searches direct and routed tools and returns compact category/tool/description matches.
- `get_tool_info` returns one tool's real input schema and metadata.
- `execute_tool` executes a routed tool by name. First use `search_tools` or `get_category_tools` to discover the tool, then call `get_tool_info` to obtain its exact input schema.

Example routed call:

```json
{
  "name": "get_lcd_status",
  "arguments": {}
}
```

Without `--mcp-router`, call every MCP tool directly.

## Quick Start

Get a desktop build of Gearboy from the [downloads](README.md#downloads) or [build it from source](README.md#build-instructions). In the examples below, replace `/path/to/gearboy` with the full path to the executable:

- **macOS:** `/Applications/Gearboy.app/Contents/MacOS/gearboy`
- **Linux:** `/path/to/gearboy`
- **Windows:** `C:/path/to/gearboy.exe`

### STDIO Mode with Claude Code

1. **Add the Gearboy MCP server** using the CLI:

   ```bash
   claude mcp add --transport stdio --scope user gearboy -- /path/to/gearboy --mcp-stdio
   ```

   Everything after `--` is passed to Gearboy untouched. `--scope user` makes the server available in all your projects. Use `--scope local` (the default) to add it only to the current project, or `--scope project` to share it through a `.mcp.json` file in the project root:

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

   Claude Code asks for approval the first time it uses a server from `.mcp.json`.

2. **Verify the server was added**:

   ```bash
   claude mcp list
   ```

3. **Start debugging**: Start Claude Code and ask about the game state. Gearboy launches when the session starts. Run `/mcp` inside Claude Code to check the server status or reconnect it.

### STDIO Mode with Codex

The Codex CLI, IDE extension, and desktop app share the same MCP configuration, so you only need to add the server once.

1. **Add the Gearboy MCP server** using the CLI:

   ```bash
   codex mcp add gearboy -- /path/to/gearboy --mcp-stdio
   ```

   Or add it to `~/.codex/config.toml`:

   ```toml
   [mcp_servers.gearboy]
   command = "/path/to/gearboy"
   args = ["--mcp-stdio"]
   ```

   To use the server only in one project, add the same table to `.codex/config.toml` in that project. Codex loads project configuration only for trusted projects. In TOML, write Windows paths with forward slashes or in single quotes.

2. **Verify the server was added**:

   ```bash
   codex mcp list
   ```

3. **Start debugging**: Start Codex and ask about the game state. Gearboy launches when the session starts. Type `/mcp` in the Codex CLI to see the active servers.

### HTTP Mode

1. **Start the emulator manually** with HTTP transport:

   ```bash
   ./gearboy --mcp-http
   ```

   The default endpoint is `http://127.0.0.1:7777/mcp`.

   To use a custom port:

   ```bash
   ./gearboy --mcp-http --mcp-http-port 3000
   ```

   To bind to a custom address, set a bearer token first:

   ```bash
   GEARBOY_MCP_HTTP_TOKEN="change-this-token" ./gearboy --mcp-http --mcp-http-address 0.0.0.0 --mcp-http-port 3000
   ```

   Clients must connect using the server's actual interface address, such as `http://192.168.1.50:3000/mcp`, not `0.0.0.0` or a spoofed loopback address.

   You can also start the server using the "MCP" menu in the GUI.

2. **Configure bearer-token authentication**:

   Set `GEARBOY_MCP_HTTP_TOKEN` before starting HTTP mode. Authentication is optional for loopback binds and required for wildcard or other non-loopback binds.

   macOS and Linux:

   ```bash
   GEARBOY_MCP_HTTP_TOKEN="change-this-token" ./gearboy --mcp-http
   ```

   Windows PowerShell:

   ```powershell
   $env:GEARBOY_MCP_HTTP_TOKEN = "change-this-token"
   .\gearboy.exe --mcp-http
   ```

   Windows Command Prompt:

   ```cmd
   set GEARBOY_MCP_HTTP_TOKEN=change-this-token
   gearboy.exe --mcp-http
   ```

3. **Configure Claude Code**:

   ```bash
   claude mcp add --transport http --scope user gearboy http://127.0.0.1:7777/mcp
   ```

   If the server uses a bearer token, add the header:

   ```bash
   claude mcp add --transport http --scope user gearboy http://127.0.0.1:7777/mcp \
     --header "Authorization: Bearer change-this-token"
   ```

   In a shared `.mcp.json`, read the token from the environment instead of committing it:

   ```json
   {
     "mcpServers": {
       "gearboy": {
         "type": "http",
         "url": "http://127.0.0.1:7777/mcp",
         "headers": {
           "Authorization": "Bearer ${GEARBOY_MCP_HTTP_TOKEN}"
         }
       }
     }
   }
   ```

4. **Or configure Codex**:

   ```bash
   codex mcp add gearboy --url http://127.0.0.1:7777/mcp
   ```

   If the server uses a bearer token, Codex reads it from an environment variable, so set `GEARBOY_MCP_HTTP_TOKEN` in the environment where Codex runs too:

   ```bash
   codex mcp add gearboy --url http://127.0.0.1:7777/mcp --bearer-token-env-var GEARBOY_MCP_HTTP_TOKEN
   ```

   The equivalent `~/.codex/config.toml` entry:

   ```toml
   [mcp_servers.gearboy]
   url = "http://127.0.0.1:7777/mcp"
   bearer_token_env_var = "GEARBOY_MCP_HTTP_TOKEN"
   ```

5. **Start a new Claude Code or Codex session** and start debugging

> **Note:** The MCP HTTP Server must be running standalone before connecting the AI client.
> **Security:** Without `GEARBOY_MCP_HTTP_TOKEN`, HTTP mode starts only on a loopback address. Wildcard and other non-loopback binds are refused. `Host` and browser `Origin` values are matched to the connection's actual destination address to prevent DNS rebinding and address spoofing.

## Usage Examples

Once configured, you can ask your AI assistant:

### Basic Commands

- "What game is currently loaded?"
- "Load the ROM at /path/to/game.gb"
- "Show me the current CPU registers"
- "Read 16 bytes from WRAM starting at offset 0x0000"
- "Set a breakpoint at address 0x0150"
- "Pause execution and show me all sprites"
- "Step through the next 5 instructions"
- "Capture a screenshot of the current frame"
- "Record a video of the next 600 frames to /path/to/clip.avi"
- "Tap the A button on the controller"

### Advanced Debugging Workflows

- "Find the VBlank interrupt handler, analyze what it does, and add symbols for all the subroutines it calls"
- "Locate the sprite update routine. Study how this game manages its OAM sprite system, explain the algorithm, and add bookmarks to key sections. Also add watches for any sprite-related variables you find"
- "There's a data decompression routine around address 0x4000. Step through it instruction by instruction, reverse engineer the compression algorithm, and explain how it works with examples"
- "Find where the game stores its level data in ROM. Analyze the data structure format, create a memory map showing each section, and add symbols for the data tables"
- "Profile the game for a few seconds, then tell me which functions use the most CPU time per frame and what they do"
- "The game is rendering corrupted graphics. Examine the LCD registers, check the VRAM contents, inspect the OAM sprite table, and diagnose what's causing the corruption. Set up watches on relevant memory addresses"

## Available MCP Tools

This is the full tool catalog. All tools are exposed directly by default. With `--mcp-router`, discover advanced tools through `search_tools` or `get_category_tools`, inspect their schemas with `get_tool_info`, then invoke them with `execute_tool`.

The server exposes tools organized in the following categories:

### Execution Control
- `debug_pause` - Pause emulation
- `debug_continue` - Resume emulation
- `debug_step_into` - Step one SM83 instruction
- `debug_step_over` - Step over subroutine calls
- `debug_step_out` - Step out of current subroutine
- `debug_step_frame` - Step one or more frames. Optional `frames` is 1-1000 (default 1). Optional `mode` is `async` (default, returns after scheduling) or `sync` (returns after all requested frames complete at VBlank). Use `mode: "sync"` when issuing dependent tool calls.
- `debug_run_to_cursor` - Continue execution until reaching specified address
- `debug_reset` - Reset emulation
- `debug_get_status` - Get debug status (paused, at_breakpoint, pc address)

### CPU & Registers
- `write_cpu_register` - Set register value (AF, BC, DE, HL, SP, PC, A, F, B, C, D, E, H, L)
- `get_cpu_status` - Get complete SM83 CPU status (registers, flags Z/N/H/C, IME, halt, CGB double speed)

### Memory Operations
- `list_memory_areas` - List all available memory areas (ROM0, ROM1, VRAM, RAM, WRAM0, WRAM1, WRAM, OAM, IO, HIRAM, and SGB areas when active: SGB_TILES, SGB_MAP, SGB_BPAL, SGB_SPAL, SGB_ATF, SGB_AMAP, SGB_EPAL)
- `read_memory` - Read from specific memory area
- `write_memory` - Write to specific memory area
- `get_memory_selection` - Get current memory selection range
- `select_memory_range` - Select a range of memory addresses
- `set_memory_selection_value` - Set all bytes in selection to specified value
- `add_memory_bookmark` - Add bookmark in memory area
- `remove_memory_bookmark` - Remove memory bookmark
- `list_memory_bookmarks` - List all bookmarks in memory area
- `add_memory_watch` - Add watch (tracked memory location)
- `remove_memory_watch` - Remove memory watch
- `list_memory_watches` - List all watches in memory area
- `memory_search_capture` - Capture memory snapshot for search comparison
- `memory_search` - Search memory with operators (<, >, ==, !=, <=, >=), compare types (previous, value, address), and data types (hex, signed, unsigned)
- `memory_find` - Find hex byte sequences (`hex_bytes`) or text (`text`, optional `case_sensitive`) in memory

### Disassembly & Debugging
- `get_disassembly` - Get SM83 disassembly for specified address range
- `add_symbol` - Add symbol (label) at specified address
- `remove_symbol` - Remove symbol
- `list_symbols` - List all defined symbols
- `lookup_symbol_by_name` - Find all exact-name symbol matches
- `lookup_symbol_at_address` - Find symbol at bank/address
- `add_disassembler_bookmark` - Add bookmark in disassembler
- `remove_disassembler_bookmark` - Remove disassembler bookmark
- `list_disassembler_bookmarks` - List all disassembler bookmarks
- `get_call_stack` - View function call hierarchy
- `get_trace_log` - Read trace logger entries using absolute sequence pagination. Responses include `total_entries`, `total_logged`, `oldest_sequence`, `start`, `next_sequence`, `count`, `overrun`, and `lines`. Omit `start` for the latest 100 retained entries, or use a negative value to start that many entries from the retained tail
- `set_trace_log` - Start or stop trace logging. Use `filters` for exact streams such as `cpu.instructions`, `lcd.interrupts`, or `serial.transfers`; omitting it enables the safe default of CPU instructions and interrupts. Storage options are `output` (`memory` or `disk`), `memory_size` (`100K` through `5M`), `disk_size` (`10MB` through `1GB`, or `unbounded`), and `output_path`. The `lcd.missed_vblank` filter logs each frame where the CPU access set with `vblank_watch_address` (hex) and `vblank_watch_operation` (`read`, `write`, or `read_write`) did not happen before VBlank (LY 144), with the number of consecutive misses; the first VBlank after starting, changing the watch, resetting, or loading a state only arms the check

### Profiler
- `set_profiler` - Start, stop, or reset the function profiler with `action` (`start`, `stop`, `reset`). `start` opens the Profiler debugger window and `stop` closes it. Statistics are only collected while the window is visible (in headless mode, while started), starting on the next frame
- `get_profiler_data` - Read profiler results: `collecting`, `window_open`, `total_cycles`, `frame_cycles`, `frames`, `function_count`, and per-function `name`, `symbol`, `bank`, `address`, `type`, `calls`, `calls_per_frame`, `inclusive_cycles`, `inclusive_percent`, `exclusive_cycles`, `exclusive_percent`, `average_cycles`, `min_cycles`, and `max_cycles`. Optional `sort` (`inclusive`, `exclusive`, `calls`, `average`, `max`; highest first), `count` (default 50, max 1000), and `filter` (name or hex address substring)

### Breakpoints
- `set_breakpoint` - Set execution, read, or write breakpoint (supports 3 memory areas: rom_ram, vram, io)
- `set_breakpoint_range` - Set breakpoint for an address range (supports 3 memory areas)
- `remove_breakpoint` - Remove breakpoint
- `list_breakpoints` - List all breakpoints
- `toggle_irq_breakpoints` - Enable or disable breaking on IRQs (VBlank, LCD STAT, Timer, Serial, Joypad)

### Hardware Status
- `get_lcd_registers` - Get all LCD registers (LCDC, STAT, SCY, SCX, LY, LYC, DMA, BGP, OBP0, OBP1, WY, WX) with decoded bit fields. Also CGB registers (KEY1, VBK, HDMA, BCPS, BCPD, OCPS, OCPD, SVBK)
- `get_lcd_status` - Get LCD status (mode 0-3, screen enabled, LY, LYC match, CGB info)
- `get_apu_status` - Get Game Boy APU status for all 4 channels (Square 1 with sweep, Square 2, Wave, Noise): volume, frequency, envelope, duty, wave RAM, panning, master volume
- `get_serial_status` - Get SB/SC registers, decoded clock mode/speed, transfer progress and timing, serial IRQ state, and distinct link-session, remote-peer, and transport diagnostics
- `reset_link_cable_metrics` - Reset local link-cable activity, synchronization, wait, and recovery counters
- `get_sgb_status` - Get Super Game Boy status: SGB active, mask mode, multiplayer state, last command, transfer state, border animation, effective palettes, and attribute map

### Sprites
- `list_sprites` - List all 40 OAM sprites with position, tile, attributes (priority, flip, palette, CGB bank)
- `get_sprite_image` - Get sprite image as base64 PNG

### Screen Capture
- `get_screenshot` - Capture current screen frame as base64 PNG
- `start_video_recording` - Start recording video and audio to an AVI file (MJPEG, uncompressed or PNG compressed video, 16-bit PCM audio). Only the resulting `file_path` is returned; the video stays on disk. Optional `file_path` (absolute; if omitted, an automatic name in the configured video recordings directory), `scale` (1-20), `aspect_ratio` (`screen`, `square`, `4:3`, `16:9`, `16:10`), and `quality` (`low`, `medium`, `high`, `lossless`, `lossless_compressed`). `lossless` is uncompressed; `lossless_compressed` stores PNG frames, much smaller but playable only with FFmpeg based players. Given options update the recording settings, same as the GUI menu. `screen` follows the display aspect ratio, which uses square pixels while debugging. When a shader preset is active outside debug mode, the shader output is recorded at the output window size and `scale` and `aspect_ratio` are ignored. Frames are recorded only while the emulator runs, so continue or step execution before stopping
- `stop_video_recording` - Stop the active recording and finalize the AVI file. Returns `file_path` and the number of recorded `frames`

### Media & State Management
- `get_media_info` - Get loaded ROM info (file path, name, MBC type, ROM/RAM size, CGB/SGB flags, battery)
- `list_recent_media` - List the 10 most recent ROM files opened by Gearboy
- `load_media` - Load ROM file (.gb, .dmg, .gbc, .cgb, .sgb, .zip). Automatically loads .sym symbol file if present
- `load_symbols` - Load debug symbols from file (.sym format with 'BANK:ADDRESS LABEL' entries)
- `list_save_state_slots` - List all 5 save state slots with information (rom name, timestamp, validity)
- `select_save_state_slot` - Select active save state slot (1-5) for save/load operations
- `save_state` - Save emulator state to currently selected slot
- `load_state` - Load emulator state from currently selected slot
- `save_state_file` - Save emulator state to an explicit file path
- `load_state_file` - Load emulator state from an explicit file path
- `set_fast_forward_speed` - Set fast forward speed multiplier (0: 1.5x, 1: 2x, 2: 2.5x, 3: 3x, 4: Unlimited)
- `toggle_fast_forward` - Toggle fast forward mode on/off
- `get_rewind_status` - Get rewind buffer status (enabled, snapshots, capacity, buffered seconds)
- `rewind_seek` - Seek to a specific rewind snapshot while paused

### Cheats
- `list_cheats` - List cheats with index, code, description, enabled and valid state
- `add_cheat` - Add an enabled Game Genie (`XXX-XXX[-XXX]`) or GameShark (`01VVLLHH`, e.g. `010FA1C6`) cheat, join codes with `+`
- `remove_cheat` - Remove a cheat by index

### Controller Input
- `controller_button` - Control a button on the Game Boy. Use action 'press' to hold the button, 'release' to let it go, or 'press_and_release' to simulate a quick tap. Buttons: up, down, left, right, a, b, start, select
- `controller_macro` - Run an ordered input macro. Top-level `player` defaults to 1, and each command may override it. Supported commands are `tap`, `press`, `release`, and `wait`; timing is explicit through `wait` frame counts
- `get_input_state` - Get effective pressed buttons and pending tap releases

## How MCP Works in Gearboy

- The MCP server runs **alongside** the GUI in a background thread
- The emulator GUI remains fully functional (you can play/debug normally while using MCP)
- Commands from the AI are queued and executed on the GUI thread
- Both GUI and MCP share the same emulator state
- Changes made through MCP are instantly reflected in the GUI and vice versa

## Architecture

### STDIO Transport
```
┌─────────────────┐                    ┌──────────────────┐
│  Claude Code /  │       stdio        │     Gearboy      │
│      Codex      │◄──────────────────►│    MCP Server    │
│   (AI Client)   │       pipes        │  (background)    │
└─────────────────┘                    └──────────────────┘
        │                                       │
        └───► Launches ►────────────────────────┘
                                                │
                                                │ Shared State
                                                ▼
                                       ┌──────────────────┐
                                       │   Emulator Core  │
                                       │   + GUI Window   │
                                       └──────────────────┘
```

### HTTP Transport
```
┌─────────────────┐                    ┌──────────────────┐
│  Claude Code /  │  HTTP (port 7777)  │     Gearboy      │
│      Codex      │◄──────────────────►│ MCP HTTP Server  │
│   (AI Client)   │                    │    (listener)    │
└─────────────────┘                    └──────────────────┘
                                                │
                                                │ Shared State
                                                ▼
                                       ┌──────────────────┐
                                       │   Emulator Core  │
                                       │   + GUI Window   │
                                       └──────────────────┘
```
