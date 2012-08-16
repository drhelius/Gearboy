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
    bool Tick(u8 clockCycles, u8* pFrameBuffer);
    void EnableScreen();
    void DisableScreen();
    bool IsScreenEnabled();
private:
    void ScanLine(int line);
    void RenderBG(int line);
    void RenderWindow(int line);
    void RenderSprites(int line);
    void UpdateStatRegister();
    void UpdateLYRegister();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    int* m_pSpriteXCacheBuffer;
    u8* m_pColorCacheBuffer;
    int m_iStatusMode;
    int m_iStatusModeCounter;
    int m_iStatusModeCounterAux;
    int m_iStatusModeLYCounter;
    int m_iScreenEnableDelayCycles;
    bool m_bScreenEnabled;
};

#endif	/* VIDEO_H */

