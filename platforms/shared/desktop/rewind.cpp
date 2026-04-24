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

#include <string.h>
#include "emu.h"
#include "config.h"
#include "gearboy.h"
#include "events.h"

#define REWIND_IMPORT
#include "rewind.h"

static u8* buffer = NULL;
static size_t sizes[REWIND_MAX_SNAPSHOTS] = { 0 };
static int head = 0;
static int count = 0;
static int capacity = 0;
static int frame_accum = 0;
static bool active = false;
static bool storage_dirty = true;
static int seek_age = -1;
static size_t slot_size = 0;
static size_t allocated_size = 0;

static int slot_at(int age);
static int get_target_capacity(void);
static size_t get_target_slot_size(void);
static bool ensure_storage(void);
static void truncate_to_seek_position(void);
static void restore_screenshot(const u8* slot, size_t size);

bool rewind_init(void)
{
    rewind_reset();
    return true;
}

void rewind_destroy(void)
{
    SafeDeleteArray(buffer);
    allocated_size = 0;
    slot_size = 0;
    capacity = 0;
    count = 0;
    head = 0;
    frame_accum = 0;
    active = false;
    storage_dirty = true;
    seek_age = -1;
}

void rewind_reset(void)
{
    capacity = get_target_capacity();
    head = 0;
    count = 0;
    frame_accum = 0;
    active = false;
    storage_dirty = true;
    seek_age = -1;
    for (int i = 0; i < REWIND_MAX_SNAPSHOTS; i++)
        sizes[i] = 0;

    if (!emu_is_empty())
        ensure_storage();
}

void rewind_push(void)
{
    if (!config_rewind.enabled)
        return;
    if (!IsValidPointer(buffer))
        return;
    if (emu_is_empty() || emu_is_paused())
        return;
    if (active)
        return;

    if (!ensure_storage())
        return;

    frame_accum++;
    if (frame_accum < config_rewind.frames_per_snapshot)
        return;
    frame_accum = 0;

    u8* slot = buffer + ((size_t)head * slot_size);
    size_t size = slot_size;

    if (!emu_get_core()->SaveState(slot, size, true))
    {
        Log("Rewind: failed to save snapshot into %zu-byte slot", slot_size);
        return;
    }

    sizes[head] = size;
    head = (head + 1) % capacity;
    if (count < capacity)
        count++;
}

bool rewind_pop(void)
{
    if (count == 0)
        return false;
    if (!IsValidPointer(buffer))
        return false;

    int idx = slot_at(0);
    const u8* slot = buffer + ((size_t)idx * slot_size);
    size_t size = sizes[idx];

    bool ok = emu_get_core()->LoadState(slot, size);

    if (ok)
    {
        restore_screenshot(slot, size);
        events_sync_input();
    }

    head = idx;
    count--;
    seek_age = -1;
    return ok;
}

void rewind_commit_seek(void)
{
    truncate_to_seek_position();
}

void rewind_set_active(bool a)
{
    active = a;
    if (!a)
        frame_accum = 0;
}

bool rewind_is_active(void)
{
    return active;
}

int rewind_get_snapshot_count(void)
{
    return count;
}

size_t rewind_get_memory_usage(void)
{
    return allocated_size;
}

bool rewind_seek(int age)
{
    if (age < 0 || age >= count)
        return false;
    if (!IsValidPointer(buffer))
        return false;

    int idx = slot_at(age);
    const u8* slot = buffer + ((size_t)idx * slot_size);
    size_t size = sizes[idx];

    bool ok = emu_get_core()->LoadState(slot, size);

    if (ok)
    {
        restore_screenshot(slot, size);
        events_sync_input();
        seek_age = age;
    }

    return ok;
}

int rewind_get_capacity(void)
{
    return get_target_capacity();
}

int rewind_get_frames_per_snapshot(void)
{
    return config_rewind.frames_per_snapshot;
}

static int slot_at(int age)
{
    int idx = head - 1 - age;
    while (idx < 0)
        idx += capacity;
    return idx;
}

static int get_target_capacity(void)
{
    int fps = config_rewind.frames_per_snapshot;
    if (fps < 1)
        fps = 1;

    int target = (config_rewind.buffer_seconds * 60 + fps - 1) / fps;
    if (target < 1)
        target = 1;
    if (target > REWIND_MAX_SNAPSHOTS)
        target = REWIND_MAX_SNAPSHOTS;

    return target;
}

static size_t get_target_slot_size(void)
{
    if (emu_is_empty())
        return 0;

    size_t target_slot_size = 0;
    if (!emu_get_core()->SaveState(NULL, target_slot_size, true))
        return 0;

    return target_slot_size;
}

static bool ensure_storage(void)
{
    int target_capacity = get_target_capacity();
    size_t target_slot_size = get_target_slot_size();
    if (target_slot_size == 0)
        return false;

    if (!storage_dirty && IsValidPointer(buffer) && (capacity == target_capacity) && (slot_size >= target_slot_size))
        return true;

    if (storage_dirty && IsValidPointer(buffer) && (capacity == target_capacity) && (slot_size >= target_slot_size))
    {
        storage_dirty = false;
        return true;
    }

    size_t target_size = (size_t)target_capacity * target_slot_size;
    u8* new_buffer = new (std::nothrow) u8[target_size];
    if (!IsValidPointer(new_buffer))
    {
        Log("Rewind: failed to allocate %zu bytes", target_size);
        return false;
    }

    SafeDeleteArray(buffer);
    buffer = new_buffer;
    allocated_size = target_size;
    slot_size = target_slot_size;
    capacity = target_capacity;
    head = 0;
    count = 0;
    frame_accum = 0;
    active = false;
    storage_dirty = false;
    seek_age = -1;
    for (int i = 0; i < REWIND_MAX_SNAPSHOTS; i++)
        sizes[i] = 0;

    Log("Rewind: allocated %.1f MB ring buffer (%d snapshots, %zu-byte slots)",
        (double)target_size / (1024.0 * 1024.0), target_capacity, target_slot_size);

    return true;
}

static void truncate_to_seek_position(void)
{
    if (seek_age <= 0)
    {
        seek_age = -1;
        return;
    }

    int idx = slot_at(seek_age);
    head = (idx + 1) % capacity;
    count -= seek_age;
    seek_age = -1;
}

static void restore_screenshot(const u8* slot, size_t size)
{
    if (size <= sizeof(GB_SaveState_Header))
        return;

    const GB_SaveState_Header* header = reinterpret_cast<const GB_SaveState_Header*>(
        slot + size - sizeof(GB_SaveState_Header));

    if (header->magic != GB_SAVESTATE_MAGIC)
        return;
    if (header->screenshot_size == 0)
        return;

    size_t max_screenshot_size = (size_t)SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT * sizeof(GB_Color);
    if (header->screenshot_size > max_screenshot_size)
        return;

    if (header->screenshot_size > (size - sizeof(GB_SaveState_Header)))
        return;

    size_t screenshot_offset = size - sizeof(GB_SaveState_Header) - header->screenshot_size;
    const u8* screenshot_data = slot + screenshot_offset;

    memcpy(emu_frame_buffer, screenshot_data, header->screenshot_size);
}