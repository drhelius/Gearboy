#include "Cartridge.h"

Cartridge::Cartridge()
{
    Reset();
}

Cartridge::~Cartridge()
{
    SafeDeleteArray(m_pTheROM);
}

void Cartridge::Reset()
{
    SafeDeleteArray(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_iType = 0;
    m_bValidROM = false;
}

bool Cartridge::IsValidROM() const
{
    return m_bValidROM;
}

int Cartridge::GetType() const
{
    return m_iType;
}

int Cartridge::GetRAMSize() const
{
    return m_iRAMSize;
}

int Cartridge::GetROMSize() const
{
    return m_iROMSize;
}

const char* Cartridge::GetName() const
{
    return m_szName;
}

int Cartridge::GetTotalSize() const
{
    return m_iTotalSize;
}

u8* Cartridge::GetTheROM() const
{
    return m_pTheROM;
}

void Cartridge::LoadFromFile(const char* path)
{
    using namespace std;

    ifstream file(path, ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
        ifstream::pos_type size = file.tellg();
        char* memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        LoadFromBuffer(reinterpret_cast<u8*> (memblock), size);

        SafeDeleteArray(memblock);
    }
    else
    {
        //error
    }
}

void Cartridge::LoadFromBuffer(const u8* buffer, int size)
{
    if (IsValidPointer(buffer))
    {
        m_iTotalSize = size;
        m_pTheROM = new u8[m_iTotalSize];
        memcpy(m_pTheROM, buffer, m_iTotalSize);

        GatherMetadata();
    }
}

void Cartridge::GatherMetadata()
{
    char name[16] = {0};

    for (int i = 0; i < 16; i++)
    {
        name[i] = m_pTheROM[0x0134 + i];
        
        if (name[i] == 0)
        {
            break;
        }
    }
    
    strcpy(m_szName, name);

    m_iType = m_pTheROM[0x147];
    m_iROMSize = m_pTheROM[0x148];
    m_iRAMSize = m_pTheROM[0x149];

    int checksum = 0;

    for (int j = 0x134; j < 0x14E; j++)
    {
        checksum += m_pTheROM[j];
    }

    m_bValidROM = ((checksum + 25) & BIT_MASK_8) == 0;

    //  only accept 32kb cartridges until mbc code is written
    if ((m_iType != 0) || (m_iROMSize != 0) || (m_iRAMSize != 0))
    {
        m_bValidROM = false;
    }
}

