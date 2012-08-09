#include "Video.h"

Video::Video()
{
    InitPointer(m_pFrameBuffer);
}

Video::~Video()
{
    SafeDeleteArray(m_pFrameBuffer);
}

void Video::Init()
{
    m_pFrameBuffer = new u8[256 * 256];
    
    Reset();
}

void Video::Reset()
{
    for (int i = 0; i < (256 * 256); i++)
        m_pFrameBuffer[i] = 0;
}

const u8* Video::GetFrameBuffer() const
{
    return m_pFrameBuffer;
}

void Video::Tick()
{

}

