#include <iostream>
#include <fstream>
#include "Memory.h"
#include "MemoryRule.h"
#include "MemoryCell.h"

Memory::Memory()
{
    InitPointer(m_pMap);
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
}

void Memory::Init()
{
    m_pMap = new MemoryCell[65536];

    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 65536; i++)
    {
        m_pMap[i].Reset();
        if (i >= 0xFF00)
            m_pMap[i].SetValue(kInitialValuesForFFXX[i - 0xFF00]);
    }
}

void Memory::AddRule(MemoryRule* pRule)
{
    m_Rules.push_back(pRule);
}

u8 Memory::Read(u16 address)
{
    return Retrieve(address);
}

void Memory::Write(u16 address, u8 value)
{
    Load(address, value);
}

u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address].Read();
}

void Memory::Load(u16 address, u8 value)
{
    m_pMap[address].Write(value);
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    m_pMap[address].SetDisassembledString(szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pMap[address].GetDisassembledString()[0] != 0;
}

void Memory::LoadBank0FromROM(u8* pTheROM)
{
    // loads the first 32KB only (bank 0)

    for (int i = 0; i < 0x8000; i++)
    {
        m_pMap[i].SetValue(pTheROM[i]);
    }
}

void Memory::MemoryDump(const char* szFilePath)
{
    using namespace std;

    ofstream myfile(szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 65536; i++)
        {
            if (IsDisassembled(i))
            {
                myfile << "0x" << hex << i << "\t " << m_pMap[i].GetDisassembledString() << "\n";
            }
            else
            {
                myfile << "0x" << hex << i << "\t [0x" << hex << (int)m_pMap[i].GetValue() << "]\n";
            }
        }

        myfile << "This is a line.\n";
        myfile << "This is another line.\n";
        myfile.close();
    }
}

