#ifndef MEMORY_H
#define	MEMORY_H

#include "definitions.h"
#include <vector>

class MemoryRule;
class MemoryCell;

class Memory
{
public:
    Memory();
    ~Memory();
    void AddRule(MemoryRule* pRule);
    u8 Read(u16 address);
    void Write(u16 address, u8 value);
    
private:
    MemoryCell* m_pMap;
    std::vector<MemoryRule*> m_Rules;
};

#endif	/* MEMORY_H */

