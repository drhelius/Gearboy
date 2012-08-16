#ifndef VIDEO_H
#define	VIDEO_H

#include <vector>
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
    void SortSprites();
    void UpdateStatRegister();
    void UpdateLYRegister();
private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    u8* m_pBackgroundColorBuffer;
    int m_iStatusMode;
    int m_iStatusModeCounter;
    int m_iStatusModeCounterAux;
    int m_iStatusModeLYCounter;
    bool m_bScreenEnabled;
    std::vector< std::pair<int, int> > m_SortedSpritesVector;
};

struct sprite_sort_x {
    bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
        return left.second > right.second;
    }
};

struct sprite_sort_oam {
    bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
        return left.first > right.first;
    }
};
#endif	/* VIDEO_H */

