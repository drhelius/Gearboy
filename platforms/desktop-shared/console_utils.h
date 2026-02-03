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

#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <cstring>

inline bool attach_parent_console(int argc, char* argv[])
{
    bool is_mcp_stdio = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--mcp-stdio") == 0)
        {
            is_mcp_stdio = true;
            break;
        }
    }

    if (is_mcp_stdio)
        return false;

    if (AttachConsole(ATTACH_PARENT_CONSOLE))
    {
        FILE* fp_stdout = nullptr;
        FILE* fp_stderr = nullptr;

        freopen_s(&fp_stdout, "CONOUT$", "w", stdout);
        freopen_s(&fp_stderr, "CONOUT$", "w", stderr);

        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        return true;
    }

    return false;
}

#else

inline bool attach_parent_console(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return false;
}

#endif

#endif /* CONSOLE_UTILS_H */
