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
#include "GearboyCore.h"
#include "Input.h"

bool g_mcp_stdio_mode = true;

static void Check(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static u8 ReadButtons(Input* input, int player)
{
    input->SetCurrentPlayer(player);
    for (int i = 0; i < 20; i++)
        input->Tick(10000);
    return input->Read() & 0x0F;
}

int main()
{
    GearboyCore core;
    core.Init();
    Input* input = core.GetInput();
    input->Reset();
    input->Write(0x10);

    core.KeyPressed(A_Key, 1);
    Check(ReadButtons(input, 0) == 0x0F, "player 1 does not see player 2 input");
    Check(ReadButtons(input, 1) == 0x0E, "player 2 sees its A button");

    core.KeyPressed(Start_Key, 3);
    Check(ReadButtons(input, 3) == 0x07, "player 4 sees its Start button");
    Check(ReadButtons(input, 2) == 0x0F, "player 3 is unaffected");

    core.KeyReleased(A_Key, 1);
    Check(ReadButtons(input, 1) == 0x0F, "player 2 releases A");

    core.KeyPressed(B_Key);
    Check(ReadButtons(input, 0) == 0x0D, "default player is player 1");

    printf("SGB multiplayer tests passed\n");
    return 0;
}
