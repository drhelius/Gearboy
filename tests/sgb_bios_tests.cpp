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
#include <vector>
#include "GearboyCore.h"
#include "SGB.h"
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "miniz.h"

bool g_mcp_stdio_mode = true;

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static const u16 kPalette[4] = { 0x67BF, 0x265B, 0x10B5, 0x2866 };

static std::vector<u8> BuildFakeBIOS()
{
    std::vector<u8> rom(0x20000, 0);
    memcpy(&rom[0x7FC0], "Super GAMEBOY", 13);
    for (int i = 0; i < 4; i++)
    {
        rom[0x10000 + i * 2] = kPalette[i] & 0xFF;
        rom[0x10001 + i * 2] = kPalette[i] >> 8;
    }
    return rom;
}

static bool LoadDMGROM(GearboyCore& core)
{
    std::vector<u8> rom(0x8000, 0);
    rom[0x0100] = 0x18; // JR -2
    rom[0x0101] = 0xFE;
    return core.LoadROMFromBuffer(rom.data(), (int)rom.size(), false);
}

// Optional real BIOS check: GEARBOY_SGB_BIOS_DIR must hold SGB1.sfc and SGB2.sfc
static void CheckRealBIOS(const char* dir, const char* name, u32 expected_border_crc)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Skipping %s: not found\n", path);
        return;
    }
    std::vector<u8> rom(0x100000);
    int size = (int)fread(rom.data(), 1, rom.size(), file);
    fclose(file);

    u16 palette[4];
    SGB::Border border;
    bool has_border = false;
    Check(SGB::ParseBIOS(rom.data(), size, palette, &border, &has_border), "real BIOS is recognized");
    Check(has_border, "real BIOS provides a border");
    Check(memcmp(palette, kPalette, sizeof(kPalette)) == 0, "real BIOS palette 1-A");
    u32 crc = (u32)mz_crc32(MZ_CRC32_INIT, reinterpret_cast<const u8*>(&border), sizeof(border));
    if (crc != expected_border_crc)
        fprintf(stderr, "%s border crc 0x%08X\n", name, crc);
    Check(crc == expected_border_crc, "real BIOS border matches reference");
}

int main()
{
    std::vector<u8> bios = BuildFakeBIOS();

    {
        GearboyCore core;
        core.Init();
        Check(LoadDMGROM(core), "load DMG ROM without BIOS");
        Check(!core.IsSGB(), "no BIOS keeps DMG games in DMG mode");
    }

    {
        GearboyCore core;
        core.Init();
        core.LoadSGBBIOS(bios.data(), (int)bios.size());
        Check(core.HasSGBBIOS(), "fake BIOS palette is accepted");
        Check(!core.HasSGBBIOSBorder(), "unknown BIOS revision has no border");
        Check(LoadDMGROM(core), "load DMG ROM with BIOS");
        Check(!core.IsSGB(), "DMG games stay in DMG mode unless all games is enabled");

        core.SetSGBAllGames(true);
        Check(LoadDMGROM(core), "reload DMG ROM with all games");
        Check(core.IsSGB(), "all games runs DMG games in SGB mode");
        const u16* effective = core.GetSGB()->GetEffectivePalettes();
        for (int p = 0; p < 4; p++)
            Check(memcmp(&effective[p * 4], kPalette, sizeof(kPalette)) == 0, "BIOS palette applied to all palettes");

        core.SetSGBEnabled(false);
        Check(LoadDMGROM(core), "reload DMG ROM with SGB disabled");
        Check(!core.IsSGB(), "SGB disabled wins over all games");
    }

    {
        GearboyCore core;
        core.Init();
        std::vector<u8> bad = bios;
        bad[0x7FC0] = 'X';
        core.LoadSGBBIOS(bad.data(), (int)bad.size());
        Check(!core.HasSGBBIOS(), "wrong title is rejected");
        core.LoadSGBBIOS(bios.data(), 0x10000);
        Check(!core.HasSGBBIOS(), "truncated BIOS is rejected");
        core.LoadSGBBIOS(NULL, 0);
        Check(!core.HasSGBBIOS(), "NULL BIOS clears the defaults");
    }

    const char* dir = getenv("GEARBOY_SGB_BIOS_DIR");
    if (dir)
    {
        CheckRealBIOS(dir, "SGB1.sfc", 0x2FEE481C);
        CheckRealBIOS(dir, "SGB2.sfc", 0x426AE616);
    }

    printf("SGB BIOS tests passed\n");
    return 0;
}
