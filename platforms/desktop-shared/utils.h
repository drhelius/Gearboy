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

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <libgen.h>
#elif defined(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY_PATTERN_SPACED "%c%c%c%c %c%c%c%c"
#define BYTE_TO_BINARY_PATTERN_ALL_SPACED "%c %c %c %c %c %c %c %c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

static inline void get_executable_path(char* path, size_t size)
{
#if defined(_WIN32)
    DWORD len = GetModuleFileNameA(NULL, path, (DWORD)size);
    if (len > 0 && len < size) {
        char* last_slash = strrchr(path, '\\');
        if (last_slash) *last_slash = '\0';
    } else {
        path[0] = '\0';
    }
#elif defined(__APPLE__)
    uint32_t bufsize = (uint32_t)size;
    if (_NSGetExecutablePath(path, &bufsize) == 0) {
        char* dir = dirname(path);
        strncpy(path, dir, size);
        path[size-1] = '\0';
    } else {
        path[0] = '\0';
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", path, size-1);
    if (len != -1) {
        path[len] = '\0';
        char* last_slash = strrchr(path, '/');
        if (last_slash) *last_slash = '\0';
    } else {
        path[0] = '\0';
    }
#else
    (void)(size);
    path[0] = '\0';
#endif
}

#endif /* UTILS_H */