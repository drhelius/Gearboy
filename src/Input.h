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
    u8 GetJoyPadState();
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8 m_JoypadState;
};

#endif	/* INPUT_H */

