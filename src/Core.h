#ifndef CORE_H
#define	CORE_H

#include "definitions.h"

class Memory;
class Processor;
class Video;
class Audio;
class Cartridge;

class Core
{
public:
    Core();
    ~Core();
    void Reset();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Video* m_pVideo;
    Audio* m_pAudio;
    Cartridge* m_pCartridge;
};

#endif	/* CORE_H */

