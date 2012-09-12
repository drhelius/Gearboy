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

#ifndef __Gearboy__EmulatorInput__
#define __Gearboy__EmulatorInput__

#include "inputmanager.h"

@class Emulator;

class EmulatorInput
{
public:
    EmulatorInput(Emulator* pEmulator);
    ~EmulatorInput();
    void InputController(stInputCallbackParameter parameter, int id);
    void InputButtons(stInputCallbackParameter parameter, int id);
    void Init();
    
private:
    InputCallback<EmulatorInput>* m_pInputCallbackController;
    InputCallback<EmulatorInput>* m_pInputCallbackButtons;
    Emulator* m_pEmulator;
    bool m_bController[4];
};

#endif /* defined(__Gearboy__EmulatorInput__) */
