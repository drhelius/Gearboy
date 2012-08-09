#include "Video.h"
#include "Memory.h"

Video::Video(Memory* pMemory)
{
    m_pMemory = pMemory;
    InitPointer(m_pFrameBuffer);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_byStatusModeLYCounter = 0;
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
    m_iStatusMode = 1;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_byStatusModeLYCounter = 144;

    for (int i = 0; i < (256 * 256); i++)
        m_pFrameBuffer[i] = 0;
}

const u8* Video::GetFrameBuffer() const
{
    return m_pFrameBuffer;
}

bool Video::Tick(u8 clockCycles)
{
    bool vblank = false;
    
    m_iStatusModeCounter += clockCycles;

    switch (m_iStatusMode)
    {
        case 0:
        {
            if (m_iStatusModeCounter >= 204)
            {
                m_iStatusModeCounter -= 204;
                m_iStatusMode = 2;
                
                ScanLine(m_byStatusModeLYCounter);
                m_byStatusModeLYCounter++;

                UpdateLYRegister();

                if (m_byStatusModeLYCounter == 144)
                {
                    m_iStatusModeCounter = 0;
                    m_iStatusMode = 1;

                    // Enable V-Blank interrupt flag
                    m_pMemory->Load(0xFF0F, m_pMemory->Retrieve(0xFF0F) | 0x01);
                    
                    vblank = true;
                }

                UpdateStatRegister();
            }
            break;
        }

        case 1:
        {
            m_iStatusModeCounterAux += clockCycles;

            if (m_iStatusModeCounterAux >= 456)
            {
                m_byStatusModeLYCounter++;
                m_iStatusModeCounterAux = 0;

                UpdateLYRegister();
            }

            if (m_iStatusModeCounter >= 4560)
            {
                m_iStatusModeCounter = 0;
                m_iStatusModeCounterAux = 0;
                m_iStatusMode = 2;
                m_byStatusModeLYCounter = 0;

                UpdateStatRegister();
                UpdateLYRegister();
            }
            break;
        }

        case 2:
        {
            if (m_iStatusModeCounter >= 80)
            {
                m_iStatusModeCounter -= 80;
                m_iStatusMode = 3;

                UpdateStatRegister();
            }
            break;
        }

        case 3:
        {
            if (m_iStatusModeCounter >= 172)
            {
                m_iStatusModeCounter -= 172;
                m_iStatusMode = 0;

                // Enable LCDC interrupt flag
                m_pMemory->Load(0xFF0F, m_pMemory->Retrieve(0xFF0F) | 0x2);

                UpdateStatRegister();
            }
            break;
        }
    }

    return vblank;
}

void Video::ScanLine(int line)
{

}

void Video::UpdateStatRegister()
{
    // Updates the STAT register with current mode
    m_pMemory->Load(0xFF41, m_pMemory->Retrieve(0xFF41) & 0xFC);
    m_pMemory->Load(0xFF41, m_pMemory->Retrieve(0xFF41) | (m_iStatusMode & 0x3));
}

void Video::UpdateLYRegister()
{
    // Establish the LY register
    m_pMemory->Load(0xFF44, m_byStatusModeLYCounter);
}
