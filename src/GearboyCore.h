#ifndef CORE_H
#define	CORE_H

#include "definitions.h"

class Memory;
class Processor;
class Video;
class Audio;
class Cartridge;
class IORegistersMemoryRule;

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
private:
    void InitMemoryRules();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Video* m_pVideo;
    Audio* m_pAudio;
    Cartridge* m_pCartridge;
    IORegistersMemoryRule* m_pIORegistersMemoryRule;
};

#endif	/* CORE_H */

