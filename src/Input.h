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

#ifndef INPUT_H
#define	INPUT_H

#include "definitions.h"

class Memory;
class Processor;

class Input
{
public:
    Input(Memory* pMemory, Processor* pProcessor);
    void Init();
    void Reset();
    void Tick(unsigned int clockCycles);
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
    void Write(u8 value);
    u8 Read();

private:
    void Update();

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8 m_JoypadState;
    u8 m_P1;
    int m_iInputCycles;
};

inline void Input::Tick(unsigned int clockCycles)
{
    m_iInputCycles += clockCycles;

    // Joypad Poll Speed (64 Hz)
    if (m_iInputCycles >= 65536)
    {
        m_iInputCycles -= 65536;
        Update();
    }
}

inline void Input::Write(u8 value)
{
    m_P1 = (m_P1 & 0xCF) | (value & 0x30);
    Update();
}

inline u8 Input::Read()
{
    return m_P1;
}

#endif	/* INPUT_H */

