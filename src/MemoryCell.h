#ifndef MEMORYCELL_H
#define	MEMORYCELL_H

#include "definitions.h"
#include "EightBitRegister.h"

class MemoryCell
{
public:
    MemoryCell();
    void Reset();
    u8 Read();
    void Write(u8 value);
    void SetDisassembledString(const char* szDisassembled);
    char* GetDisassembledString();
    int GetReadCount() const;
    int GetWriteCount() const;
    u8 GetOldValue() const;
    u8 GetValue() const;
private:
    EightBitRegister m_Register;
    EightBitRegister m_OldRegister;
    int m_iWriteCount;
    int m_iReadCount;
    char m_szDisassembled[MAX_STRING_SIZE];
};

#endif	/* MEMORYCELL_H */

