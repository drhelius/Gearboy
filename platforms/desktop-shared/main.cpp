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

#include "../../src/gearboy.h"
#include "application.h"

int main(int argc, char* argv[])
{
    char* rom_file = NULL;
    char* symbol_file = NULL;
    bool show_usage = false;
    int ret = 0;

    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-?") == 0) ||
            (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "/?") == 0))
        {
            show_usage = true;
            ret = 0;
        } 
        else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0))
        {
            printf("%s %s\n", GEARBOY_TITLE, GEARBOY_VERSION);
            printf("Build: %s\n", EMULATOR_BUILD);
            return 0;
        }
        else if (argv[i][0] == '-')
        {
            show_usage = true;
            ret = -1;
        }
    }

    switch (argc)
    {
        case 3:
            rom_file = argv[1];
            symbol_file = argv[2];
            break;
        case 2:
            rom_file = argv[1];
            break;
        case 1:
            break;
        default:
            show_usage = true;
            ret = -1;
            break;
    }

    if (show_usage)
    {
        printf("Usage: %s [rom_file] [symbol_file]\n", argv[0]);
        return ret;
    }

    ret = application_init(rom_file, symbol_file);

    if (ret == 0)
        application_mainloop();

    application_destroy();    

    return ret;
}
