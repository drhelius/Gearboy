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

#include "emu.h"
#include "config.h"
#include "gearboy.h"

#define RUNAHEAD_IMPORT
#include "runahead.h"

static u8* runahead_buffer = NULL;
static s16* runahead_audio = NULL;
static size_t runahead_buffer_size = 0;

static bool ensure_buffer(void);

void runahead_init(void)
{
    runahead_audio = new s16[AUDIO_BUFFER_SIZE];
    runahead_buffer = NULL;
    runahead_buffer_size = 0;
}

void runahead_destroy(void)
{
    SafeDeleteArray(runahead_audio);
    SafeDeleteArray(runahead_buffer);
    runahead_buffer_size = 0;
}

int runahead_get_frames(void)
{
    int frames = config_emulator.runahead;

    if ((frames <= 0) || config_emulator.ffwd)
        return 0;

    return frames;
}

void runahead_run(int frames, u16* frame_buffer, s16* sample_buffer, int* sample_count)
{
    GearboyCore* core = emu_get_core();

    // Run the authoritative frame, keeping its audio while the real state advances.
    core->RunToVBlank(frame_buffer, sample_buffer, sample_count, false, NULL);

    // Allocate the reusable snapshot buffer on first use.
    if (!IsValidPointer(runahead_buffer) && !ensure_buffer())
        return;

    size_t saved_size = runahead_buffer_size;
    if (!core->SaveState(runahead_buffer, saved_size, false))
    {
        // The state outgrew the buffer. Grow it once and skip speculation this
        // frame; later frames reuse the larger buffer.
        ensure_buffer();
        return;
    }

    // Run the speculative frames with the same input, discarding their audio
    // and keeping only the last rendered frame.
    for (int i = 0; i < frames; i++)
    {
        int discarded_samples = 0;
        core->RunToVBlank(frame_buffer, runahead_audio, &discarded_samples, false, NULL);
    }

    // Roll back to the authoritative frame. If restoring ever fails, the
    // authoritative state is unrecoverable, so keep the (valid) speculative
    // state as the new timeline and disable run-ahead. Emulation continues
    // without interruption.
    if (!core->LoadState(runahead_buffer, saved_size))
    {
        Log("Run-ahead: failed to restore state, disabling run-ahead");
        config_emulator.runahead = 0;
    }
}

static bool ensure_buffer(void)
{
    size_t needed = 0;
    if (!emu_get_core()->SaveState(NULL, needed, false) || (needed == 0))
        return false;

    // The buffer is allocated once and only ever grows, so it is reused every
    // frame without per-frame allocations.
    if (IsValidPointer(runahead_buffer) && (runahead_buffer_size >= needed))
        return true;

    u8* new_buffer = new (std::nothrow) u8[needed];
    if (!IsValidPointer(new_buffer))
    {
        Log("Run-ahead: failed to allocate %zu bytes, disabling run-ahead", needed);
        config_emulator.runahead = 0;
        return false;
    }

    SafeDeleteArray(runahead_buffer);
    runahead_buffer = new_buffer;
    runahead_buffer_size = needed;
    return true;
}
