#ifndef CORE_H
#define	CORE_H

#include "definitions.h"

class Memory;
class Processor;
class Video;
class Audio;
class Input;
class Cartridge;
class IORegistersMemoryRule;
class RomOnlyMemoryRule;
class MBC1MemoryRule;
class MBC2MemoryRule;
class MBC3MemoryRule;

class GearboyCore
{
public:
    GearboyCore();
    ~GearboyCore();
    void Init();
    void Reset();
    void RunToVBlank(u8* pFrameBuffer);
    void LoadROM(const char* szFilePath);
    Memory* GetMemory();
    void KeyPressed(Gameboy_Keys key);
    void KeyReleased(Gameboy_Keys key);
private:
    void InitMemoryRules();
    void AddMemoryRules();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Video* m_pVideo;
    Audio* m_pAudio;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    IORegistersMemoryRule* m_pIORegistersMemoryRule;
    RomOnlyMemoryRule* m_pRomOnlyMemoryRule;
    MBC1MemoryRule* m_pMBC1MemoryRule;
    MBC2MemoryRule* m_pMBC2MemoryRule;
    MBC3MemoryRule* m_pMBC3MemoryRule;
    Gameboy_MemoryBankControllers m_MBC;
};

#endif	/* CORE_H */

