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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include "definitions.h"

#if defined(__LIBRETRO__)
#include "libretro.h"
extern retro_log_printf_t log_cb;
#else
//extern bool g_mcp_stdio_mode;
#endif

#if defined(DEBUG_GEARBOY)
    #if defined(__ANDROID__)
        #include <android/log.h>
        #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, GEARBOY_TITLE, __VA_ARGS__);
    #endif
    #define Debug(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#else
    #define Debug(msg, ...)
#endif

#define Log(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#define Error(msg, ...) (Log_func("ERROR [%s:%d] " msg, __FILE__, __LINE__, ##__VA_ARGS__))

inline void Log_func(const char* const msg, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, 512, msg, args);
    va_end(args);

#if defined(__LIBRETRO__)
    if (log_cb)
    {
        log_cb(RETRO_LOG_INFO, "%s\n", buffer);
        return;
    }
    else
    {
        printf("%s\n", buffer);
        fflush(stdout);
        return;
    }
#else
    //if (g_mcp_stdio_mode)
    //    return;

    printf("%s\n", buffer);
    fflush(stdout);
    return;
#endif
}

#endif /* LOG_H */
