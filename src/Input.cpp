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

#include "Input.h"
#include "Memory.h"
#include "Processor.h"

Input::Input(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    memset(m_JoypadState, 0xFF, sizeof(m_JoypadState));
    m_P1 = 0xFF;
    m_iInputCycles = 0;
    m_iCurrentPlayer = 0;
}

void Input::Init()
{
    Reset();
}

void Input::Reset()
{
    memset(m_JoypadState, 0xFF, sizeof(m_JoypadState));
    m_P1 = 0xFF;
    m_iInputCycles = 0;
    m_iCurrentPlayer = 0;
}

void Input::KeyPressed(Gameboy_Keys key)
{
    m_JoypadState[0] &= ~key;
}

void Input::KeyReleased(Gameboy_Keys key)
{
    m_JoypadState[0] |= key;
}

void Input::KeyPressed(Gameboy_Keys key, int player)
{
    if (player >= 0 && player < 4)
        m_JoypadState[player] &= ~key;
}

void Input::KeyReleased(Gameboy_Keys key, int player)
{
    if (player >= 0 && player < 4)
        m_JoypadState[player] |= key;
}

void Input::SetCurrentPlayer(int player)
{
    if (player >= 0 && player < 4)
        m_iCurrentPlayer = player;
}

void Input::Update()
{
    u8 state = m_JoypadState[m_iCurrentPlayer];
    u8 current = m_P1 & 0xF0;

    switch (current & 0x30)
    {
        case 0x00:
        {
            u8 topJoypad = (state >> 4) & 0x0F;
            u8 bottomJoypad = state & 0x0F;
            current |= topJoypad & bottomJoypad;
            break;
        }
        case 0x10:
        {
            u8 topJoypad = (state >> 4) & 0x0F;
            current |= topJoypad;
            break;
        }
        case 0x20:
        {
            u8 bottomJoypad = state & 0x0F;
            current |= bottomJoypad;
            break;
        }
        case 0x30:
            current |= 0x0F;
            break;
    }

    if ((m_P1 & ~current & 0x0F) != 0)
        m_pProcessor->RequestInterrupt(Processor::Joypad_Interrupt);

    m_P1 = current;
}

void Input::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_JoypadState), sizeof(m_JoypadState));
    stream.write(reinterpret_cast<const char*> (&m_P1), sizeof(m_P1));
    stream.write(reinterpret_cast<const char*> (&m_iInputCycles), sizeof(m_iInputCycles));
}

void Input::LoadState(std::istream& stream, u32 version)
{
    using namespace std;

    if (version < 102)
    {
        u8 legacy_joypad = 0xFF;
        stream.read(reinterpret_cast<char*>(&legacy_joypad), sizeof(legacy_joypad));
        m_JoypadState[0] = legacy_joypad;
        m_JoypadState[1] = 0xFF;
        m_JoypadState[2] = 0xFF;
        m_JoypadState[3] = 0xFF;
    }
    else
    {
        stream.read(reinterpret_cast<char*>(m_JoypadState), sizeof(m_JoypadState));
    }
    stream.read(reinterpret_cast<char*>(&m_P1), sizeof(m_P1));
    stream.read(reinterpret_cast<char*>(&m_iInputCycles), sizeof(m_iInputCycles));
}
