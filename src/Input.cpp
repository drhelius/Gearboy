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
    m_JoypadState = 0xFF;
    m_P1 = 0xFF;
    m_iInputCycles = 0;
}

void Input::Init()
{
    Reset();
}

void Input::Reset()
{
    m_JoypadState = 0xFF;
    m_P1 = 0xFF;
    m_iInputCycles = 0;
}

void Input::KeyPressed(Gameboy_Keys key)
{
    m_JoypadState = UnsetBit(m_JoypadState, key);
}

void Input::KeyReleased(Gameboy_Keys key)
{
    m_JoypadState = SetBit(m_JoypadState, key);
}

void Input::Update()
{
    u8 current = m_P1 & 0xF0;

    switch (current & 0x30)
    {
        case 0x10:
        {
            u8 topJoypad = (m_JoypadState >> 4) & 0x0F;
            current |= topJoypad;
            break;
        }
        case 0x20:
        {
            u8 bottomJoypad = m_JoypadState & 0x0F;
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
