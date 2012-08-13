#ifndef VIDEO_H
#define	VIDEO_H

#include "definitions.h"

class Memory;
class Processor;

class Video
{
public:
    Video(Memory* pMemory, Processor* pProcessor);
    ~Video();
    void Init();
    void Reset();
    const u8* GetFrameBuffer() const;
    bool Tick(u8 clockCycles);
private:
    void ScanLine(int line);
    void RenderToOffscreen(int line);
    void RenderBG(int line);
    void RenderWindow(int line);
    void RenderSprites(int line);
    void UpdateStatRegister();
    void UpdateLYRegister();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    u8* m_pOffscreenBuffer;
    int m_iStatusMode;
    int m_iStatusModeCounter;
    int m_iStatusModeCounterAux;
    u8 m_byStatusModeLYCounter;
};

#endif	/* VIDEO_H */

