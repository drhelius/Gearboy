#include "Video.h"

Video::Video()
{
    m_pFrameBuffer = new u8[256 * 256]
}

Video::~Video()
{
    SafeDeleteArray(m_pFrameBuffer);
}

void Video::Reset()
{
    for (int i = 0; i < (256 * 256); i++)
        m_pFrameBuffer[i] = 0;
}

const u8* Video::GetFrameBuffer()
{
    return m_pFrameBuffer;
}

void Video::Tick()
{
    
}

