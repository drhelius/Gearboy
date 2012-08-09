#ifndef VIDEO_H
#define	VIDEO_H

#include "definitions.h"

class Video
{
public:
    Video();
    ~Video();
    void Reset();
    const u8* GetFrameBuffer() const;
    void Tick();
private:
    u8* m_pFrameBuffer;
};

#endif	/* VIDEO_H */

