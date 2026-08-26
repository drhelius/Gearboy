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

#ifndef IPS_PATCH_H
#define IPS_PATCH_H

#include "common.h"

#include <errno.h>
#include <limits.h>
#include <new>
#include <stdio.h>

static const size_t IPS_MAX_PATCH_FILE_SIZE = 64U * 1024U * 1024U;

static bool ips_build_patch_path(const char* media_path, char* patch_path, size_t patch_path_size)
{
    if (!IsValidPointer(media_path) || !IsValidPointer(patch_path) || patch_path_size == 0)
        return false;

    const char* filename = media_path;
    const char* slash = strrchr(media_path, '/');
    const char* backslash = strrchr(media_path, '\\');

    if (slash && (!backslash || slash > backslash))
        filename = slash + 1;
    else if (backslash)
        filename = backslash + 1;

    const char* dot = strrchr(filename, '.');
    size_t base_length = strlen(media_path);

    if (dot && dot != filename)
        base_length = (size_t)(dot - media_path);

    if (patch_path_size < 5 || base_length > patch_path_size - 5)
        return false;

    memcpy(patch_path, media_path, base_length);
    memcpy(patch_path + base_length, ".ips", 5);
    return true;
}

static bool ips_validate_patch(const u8* data, size_t size, size_t* working_size,
    size_t input_size, size_t* final_size, const char** error)
{
    if (size < 8)
    {
        *error = "file is too small";
        return false;
    }

    if (memcmp(data, "PATCH", 5) != 0)
    {
        *error = "invalid PATCH header";
        return false;
    }

    size_t position = 5;
    size_t maximum_end = 0;
    bool found_eof = false;

    while (position < size)
    {
        if (size - position < 3)
        {
            *error = "truncated record offset";
            return false;
        }

        u32 offset = read_u24_be(data + position);
        position += 3;

        if (offset == 0x454F46)
        {
            found_eof = true;
            break;
        }

        if (size - position < 2)
        {
            *error = "truncated record length";
            return false;
        }

        u16 length = read_u16_be(data + position);
        position += 2;
        size_t record_length = length;

        if (length == 0)
        {
            if (size - position < 3)
            {
                *error = "truncated RLE record";
                return false;
            }

            record_length = read_u16_be(data + position);
            if (record_length == 0)
            {
                *error = "zero-length RLE record";
                return false;
            }

            position += 3;
        }
        else
        {
            if (record_length > size - position)
            {
                *error = "truncated literal record";
                return false;
            }

            position += record_length;
        }

        if (record_length > SIZE_MAX - offset)
        {
            *error = "record range overflow";
            return false;
        }

        size_t record_end = (size_t)offset + record_length;
        if (record_end > maximum_end)
            maximum_end = record_end;
    }

    if (!found_eof)
    {
        *error = "missing EOF marker";
        return false;
    }

    size_t remaining = size - position;
    if (remaining != 0 && remaining != 3)
    {
        *error = "invalid data after EOF";
        return false;
    }

    *working_size = MAX(input_size, maximum_end);
    *final_size = *working_size;

    if (remaining == 3)
    {
        size_t truncate_size = read_u24_be(data + position);
        if (truncate_size < *final_size)
            *final_size = truncate_size;
    }

    if (*final_size == 0)
    {
        *error = "patch produces empty media";
        return false;
    }

    if (*working_size > INT_MAX || *final_size > INT_MAX)
    {
        *error = "patched media is too large";
        return false;
    }

    return true;
}

static void ips_apply_records(const u8* patch_data, u8* output)
{
    size_t position = 5;

    while (true)
    {
        u32 offset = read_u24_be(patch_data + position);
        position += 3;

        if (offset == 0x454F46)
            break;

        u16 length = read_u16_be(patch_data + position);
        position += 2;

        if (length == 0)
        {
            u16 rle_length = read_u16_be(patch_data + position);
            u8 value = patch_data[position + 2];
            position += 3;
            memset(output + offset, value, rle_length);
        }
        else
        {
            memcpy(output + offset, patch_data + position, length);
            position += length;
        }
    }
}

static bool ips_apply_patch(const char* media_path, const u8* input, int input_size,
    u8** output, int* output_size, char* patch_path, size_t patch_path_size)
{
    if (IsValidPointer(output))
        *output = NULL;
    if (IsValidPointer(output_size))
        *output_size = 0;
    if (IsValidPointer(patch_path) && patch_path_size > 0)
        patch_path[0] = '\0';

    if (!IsValidPointer(media_path) || !IsValidPointer(input) || input_size <= 0 ||
        !IsValidPointer(output) || !IsValidPointer(output_size) ||
        !IsValidPointer(patch_path) || patch_path_size == 0)
    {
        return false;
    }

    if (!ips_build_patch_path(media_path, patch_path, patch_path_size))
    {
        Error("Unable to build IPS patch path for %s. Loading unpatched media.", media_path);
        return false;
    }

    errno = 0;
    FILE* file = fopen_utf8(patch_path, "rb");
    if (!file)
    {
        if (errno != ENOENT && errno != ENOTDIR)
        {
            Error("Unable to open IPS patch %s: %s. Loading unpatched media.", patch_path, strerror(errno));
        }
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        Error("Unable to seek IPS patch %s. Loading unpatched media.", patch_path);
        fclose(file);
        return false;
    }

    long file_size = ftell(file);
    if (file_size <= 0)
    {
        Error("Invalid IPS patch size for %s. Loading unpatched media.", patch_path);
        fclose(file);
        return false;
    }

    if (file_size > (long)IPS_MAX_PATCH_FILE_SIZE)
    {
        Error("IPS patch %s is too large (%ld bytes). Loading unpatched media.", patch_path, file_size);
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        Error("Unable to rewind IPS patch %s. Loading unpatched media.", patch_path);
        fclose(file);
        return false;
    }

    size_t patch_size = (size_t)file_size;
    u8* patch_data = new (std::nothrow) u8[patch_size];
    if (!patch_data)
    {
        Error("Unable to allocate %zu bytes for IPS patch %s. Loading unpatched media.", patch_size, patch_path);
        fclose(file);
        return false;
    }

    size_t bytes_read = fread(patch_data, 1, patch_size, file);
    fclose(file);

    if (bytes_read != patch_size)
    {
        Error("Unable to read IPS patch %s. Loading unpatched media.", patch_path);
        SafeDeleteArray(patch_data);
        return false;
    }

    size_t working_size = 0;
    size_t final_size = 0;
    const char* error = NULL;

    if (!ips_validate_patch(patch_data, patch_size, &working_size, (size_t)input_size, &final_size, &error))
    {
        Error("Unable to apply IPS patch %s: %s. Loading unpatched media.", patch_path, error);
        SafeDeleteArray(patch_data);
        return false;
    }

    u8* patched_data = new (std::nothrow) u8[working_size];
    if (!patched_data)
    {
        Error("Unable to allocate %zu bytes for patched media %s. Loading unpatched media.", working_size, patch_path);
        SafeDeleteArray(patch_data);
        return false;
    }

    memcpy(patched_data, input, (size_t)input_size);
    if (working_size > (size_t)input_size)
        memset(patched_data + input_size, 0, working_size - (size_t)input_size);

    ips_apply_records(patch_data, patched_data);
    SafeDeleteArray(patch_data);

    *output = patched_data;
    *output_size = (int)final_size;

    Log("IPS patch applied: %s", patch_path);
    return true;
}

#endif /* IPS_PATCH_H */
