#ifndef CARTRIDGE_H
#define	CARTRIDGE_H

#include "definitions.h"

class Cartridge
{
public:
    Cartridge();
    ~Cartridge();
    void Reset();
    bool IsValidROM() const;
    int GetType() const;
    int GetRAMSize() const;
    int GetROMSize() const;
    const char* GetName() const;
    int GetTotalSize() const;
    u8* GetTheROM() const;
    void LoadFromFile(const char* path);
    void LoadFromBuffer(const u8* buffer, int size);
private:
    void GatherMetadata();
private:
    u8* m_pTheROM;
    int m_iTotalSize;
    char m_szName[MAX_STRING_SIZE];
    int m_iROMSize;
    int m_iRAMSize;
    int m_iType;
    bool m_bValidROM;
};

#endif	/* CARTRIDGE_H */

