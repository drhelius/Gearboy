#ifndef VIDEO_INLINE_H
#define VIDEO_INLINE_H

#include "Processor.h"
#include "Memory.h"

inline bool Video::Tick(unsigned int &clockCycles, u16* pColorFrameBuffer, GB_Color_Format pixelFormat)
{
    m_pColorFrameBuffer = pColorFrameBuffer;
    m_pixelFormat = pixelFormat;

    bool vblank = false;
    m_iStatusModeCounter += clockCycles;

    if (m_iPendingVBlankInterruptCycles > 0)
    {
        m_iPendingVBlankInterruptCycles -= clockCycles;

        if (m_iPendingVBlankInterruptCycles <= 0)
        {
            m_iPendingVBlankInterruptCycles = 0;
            m_pMemory->Load(0xFF0F, m_pMemory->Retrieve(0xFF0F) | Processor::VBlank_Interrupt);
            TraceEvent(TRACE_LCD_VBLANK_IRQ, 0);
        }
    }

    if (m_bScreenEnabled)
    {
        switch (m_iStatusMode)
        {
            // During H-BLANK
            case 0:
            {
                if (m_iStatusModeCounter >= 204)
                {
                    m_iStatusModeCounter -= 204;
                    m_iStatusMode = 2;
                    if (m_bCGB)
                        m_IRQ48Signal = UnsetBit(m_IRQ48Signal, 0);

                    m_iStatusModeLYCounter++;
                    m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                    CompareLYToLYC();
                    CheckWindowY();

                    if (m_iStatusModeLYCounter == 144)
                    {
                        m_iStatusMode = 1;
                        m_iStatusVBlankLine = 0;
                        m_iStatusModeCounterAux = m_iStatusModeCounter;

                        if (m_pProcessor->CGBSpeed())
                        {
                            m_iPendingVBlankInterruptCycles = 12;
                        }
                        else
                        {
                            m_pProcessor->RequestInterrupt(Processor::VBlank_Interrupt);
                            TraceEvent(TRACE_LCD_VBLANK_IRQ, 0);
                        }

                        if (m_iHideFrames > 0)
                        {
                            m_iHideFrames--;

                            if (IsValidPointer(m_pColorFrameBuffer))
                            {
                                if (m_bCGB)
                                {
                                    for (int i = 0; i < GAMEBOY_WIDTH * GAMEBOY_HEIGHT; i++)
                                        m_pColorFrameBuffer[i] = m_CGBWhiteColor;
                                }
                                else
                                {
                                    if (!m_bSGBTransferMode)
                                        memset(m_pFrameBuffer, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
                                    memset(m_pColorFrameBuffer, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT * sizeof(u16));
                                }
                            }

                            vblank = true;
                        }
                        else
                            vblank = true;

                        m_iWindowLine = 0;
                        m_bWindowYTrigger = false;
                    }

                    UpdateStatRegister();
                    RefreshStatInterruptSignal(true);
                }
                break;
            }
            // During V-BLANK
            case 1:
            {
                m_iStatusModeCounterAux += clockCycles;

                if (m_iStatusModeCounterAux >= 456)
                {
                    m_iStatusModeCounterAux -= 456;
                    m_iStatusVBlankLine++;

                    if (m_iStatusVBlankLine <= 9)
                    {
                        m_iStatusModeLYCounter++;
                        m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                        CompareLYToLYC();
                    }
                }

                if ((m_iStatusModeCounter >= 4104) && (m_iStatusModeCounterAux >= 4) && (m_iStatusModeLYCounter == 153))
                {
                    m_iStatusModeLYCounter = 0;
                    m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                    CompareLYToLYC();
                }

                if (m_iStatusModeCounter >= 4560)
                {
                    m_iStatusModeCounter -= 4560;
                    m_iStatusMode = 2;
                    CheckWindowY();
                    UpdateStatRegister();
                    RefreshStatInterruptSignal(true);
                }
                break;
            }
            // During searching OAM RAM
            case 2:
            {
                if (m_iStatusModeCounter >= 80)
                {
                    m_iStatusModeCounter -= 80;
                    m_iStatusMode = 3;
                    m_bScanLineTransfered = false;
                    UpdateStatRegister();
                    RefreshStatInterruptSignal(true);
                }
                break;
            }
            // During transfering data to LCD driver
            case 3:
            {
#ifndef PERFORMANCE
                if (m_iPixelCounter < 160 && (m_iHideFrames == 0 || m_bSGBTransferMode))
                {
                    m_iTileCycleCounter += clockCycles;
                    u8 lcdc = m_pMemory->Retrieve(0xFF40);

                    if (m_bScreenEnabled && IsSetBit(lcdc, 7))
                    {
                        while (m_iTileCycleCounter >= 3)
                        {
                            if (IsValidPointer(m_pColorFrameBuffer))
                            {
                                RenderBG(m_iStatusModeLYCounter, m_iPixelCounter);
                            }
                            m_iPixelCounter += 4;
                            m_iTileCycleCounter -= 3;

                            if (m_iPixelCounter >= 160)
                            {
                                break;
                            }
                        }
                    }
                }
#endif

                if (m_iStatusModeCounter >= 160 && !m_bScanLineTransfered)
                {
                    ScanLine(m_iStatusModeLYCounter);
                    m_bScanLineTransfered = true;
                }

                if (m_iStatusModeCounter >= 172)
                {
                    m_iPixelCounter = 0;
                    m_iStatusModeCounter -= 172;
                    m_iStatusMode = 0;
                    m_iTileCycleCounter = 0;

                    if (m_bCGB && m_pMemory->IsHDMAEnabled())
                    {
                        unsigned int cycles = m_pMemory->PerformHDMA();
                        m_iStatusModeCounter += cycles;
                        clockCycles += cycles;
                    }

                    UpdateStatRegister();
                    RefreshStatInterruptSignal(true);
                }
                break;
            }
        }
    }
    // Screen disabled
    else
    {
        if (m_iScreenEnableDelayCycles > 0)
        {
            m_iScreenEnableDelayCycles -= clockCycles;

            if (m_iScreenEnableDelayCycles <= 0)
            {
                m_iScreenEnableDelayCycles = 0;
                m_bScreenEnabled = true;
                if (m_iHideFrames < 0)
                    m_iHideFrames = 0;
                else
                    m_iHideFrames = 3;
                m_iStatusMode = 0;
                m_iStatusModeCounter = 0;
                m_iStatusModeCounterAux = 0;
                m_iPendingVBlankInterruptCycles = 0;
                m_iStatusModeLYCounter = 0;
                m_iWindowLine = 0;
                m_bWindowYTrigger = false;
                m_iStatusVBlankLine = 0;
                m_iPixelCounter = 0;
                m_iTileCycleCounter = 0;
                m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                m_IRQ48Signal = 0;

                u8 stat = m_pMemory->Retrieve(0xFF41);
                if (IsSetBit(stat, 5))
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                    m_IRQ48Signal = SetBit(m_IRQ48Signal, 2);
                    TraceEvent(TRACE_LCD_STAT_IRQ, m_IRQ48Signal);
                }

                CompareLYToLYC();
                RefreshStatInterruptSignal(true);
            }
        }
        else if (m_iStatusModeCounter >= 70224)
        {
            m_iStatusModeCounter -= 70224;
            m_iHideFrames = 0;

            if (IsValidPointer(m_pColorFrameBuffer))
            {
                if (m_bCGB)
                {
                    for (int i = 0; i < GAMEBOY_WIDTH * GAMEBOY_HEIGHT; i++)
                        m_pColorFrameBuffer[i] = m_CGBWhiteColor;
                }
                else
                {
                    if (!m_bSGBTransferMode)
                        memset(m_pFrameBuffer, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
                    memset(m_pColorFrameBuffer, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT * sizeof(u16));
                }
            }

            vblank = true;
        }
    }
    return vblank;
}

#endif /* VIDEO_INLINE_H */
