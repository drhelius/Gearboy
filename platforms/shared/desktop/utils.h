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
#include <math.h>
#include <SDL3/SDL.h>
#include <time.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif
#include "imgui.h"
#include "gearboy.h"

#ifndef Error
#define Error Log
#endif

// strncpy_fit, strncat_fit, fopen_utf8, get_date_time_string are defined in common.h

static inline void create_directory_if_not_exists(const char* path)
{
#if defined(_WIN32)
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static inline void remove_directory_and_contents(const char* path)
{
#if defined(_WIN32)
    // Not implemented for this migration
    (void)path;
#else
    DIR* dir = opendir(path);
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            remove(full_path);
        }
        closedir(dir);
        rmdir(path);
    }
#endif
}

static inline void sdl_log_error(const char* action, const char* file, int line)
{
    const char* error = SDL_GetError();
    if (error && error[0] != '\0')
    {
        Log("SDL Error: %s (%s:%d) - %s", action, file, line, error);
        SDL_ClearError();
    }
}

#define SDL_ERROR(action) sdl_log_error(action, __FILE__, __LINE__)

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

static inline int get_reset_value(int option)
{
    switch (option)
    {
        case 0:
            return -1;
        case 1:
            return 0x0000;
        case 2:
            return 0xFFFF;
        default:
            return -1;
    }
}

static inline int ends_with(const char* s, const char* suffix)
{
    size_t sl = strlen(s);
    size_t su = strlen(suffix);

    if (sl < su)
    {
        return 0;
    }

    return (memcmp(s + (sl - su), suffix, su) == 0);
}

static inline void get_executable_path(char* path, size_t size)
{
#if defined(_WIN32)
    DWORD len = GetModuleFileNameA(NULL, path, (DWORD)size);
    if (len > 0 && len < size)
    {
        char* last_slash = strrchr(path, '\\');
        if (last_slash) *last_slash = '\0';

        // Check if we're in an MCPB bundle (server\ subfolder)
        char* server_pos = strstr(path, "\\server");
        if (server_pos && (server_pos[7] == '\0' || server_pos[7] == '\\'))
        {
            *server_pos = '\0';  // Truncate at server\ to get bundle root
        }
    }
    else
    {
        path[0] = '\0';
    }
#elif defined(__APPLE__)
    uint32_t bufsize = (uint32_t)size;
    if (_NSGetExecutablePath(path, &bufsize) == 0) {
        char* dir = dirname(path);
        strncpy(path, dir, size);
        path[size - 1] = '\0';

        // Check if we're in an MCPB bundle (server/ subfolder)
        char* server_pos = strstr(path, "/server");
        if (server_pos && (server_pos[7] == '\0' || server_pos[7] == '/'))
        {
            *server_pos = '\0';  // Truncate at server/ to get bundle root
        }
        // If running inside a .app bundle, use Contents/Resources as data root
        else if (ends_with(path, "/Contents/MacOS"))
        {
            size_t len = strlen(path);
            const char* repl = "Resources";

            // Replace the trailing "MacOS" with "Resources"
            if (len >= strlen("MacOS") && (len - strlen("MacOS") + strlen(repl) + 1) <= size)
            {
                memcpy(path + (len - strlen("MacOS")), repl, strlen(repl) + 1);
            }
        }
    }
    else
    {
        path[0] = '\0';
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", path, size - 1);
    if (len != -1)
    {
        path[len] = '\0';
        char* last_slash = strrchr(path, '/');
        if (last_slash) *last_slash = '\0';

        // Check if we're in an MCPB bundle (server/ subfolder)
        char* server_pos = strstr(path, "/server");
        if (server_pos && (server_pos[7] == '\0' || server_pos[7] == '/'))
        {
            *server_pos = '\0';  // Truncate at server/ to get bundle root
        }
    }
    else
    {
        path[0] = '\0';
    }
#else
    (void)(size);
    path[0] = '\0';
#endif
}

static inline void strip_color_tags(std::string& str)
{
    size_t pos = 0;
    while ((pos = str.find('{', pos)) != std::string::npos)
    {
        size_t end_pos = str.find('}', pos);
        if (end_pos != std::string::npos)
            str.erase(pos, end_pos - pos + 1);
        else
            pos++;
    }
}

#endif /* UTILS_H */
