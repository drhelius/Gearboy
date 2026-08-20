/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef VIDEO_H
#define	VIDEO_H

#include "definitions.h"
#include "TraceLogger.h"

class Memory;
class Processor;

typedef u16 (*PaletteMatrix)[8][4][2];

class Video
{
public:
    Video(Memory* pMemory, Processor* pProcessor);
    ~Video();
    void Init();
    void Reset(bool bCGB);
    void ResetToBootromState();
    bool Tick(unsigned int &clockCycles, u16* pColorFrameBuffer, GB_Color_Format pixelFormat);
    void EnableScreen();
    void DisableScreen();
    void SetSGBTransferMode(bool enabled);
    void SetNoSpriteLimit(bool noSpriteLimit);
    bool IsScreenEnabled() const;
    const u8* GetFrameBuffer() const;
    const u16* GetColorFrameBuffer() const;
    void UpdatePaletteToSpecification(bool background, u8 value);
    void SetColorPalette(bool background, u8 value);
    bool VRAMAccessBlocked() const;
    bool CGBPaletteAccessBlocked() const;
    int GetCurrentStatusMode() const;
    void RefreshStatInterruptSignal(bool requestInterrupt);
    void ResetWindowLine();
    void CheckWindowY();
    void CompareLYToLYC();
    u8 GetIRQ48Signal() const;
    void SetIRQ48Signal(u8 signal);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream, u32 version = GB_SAVESTATE_VERSION);
    PaletteMatrix GetCGBBackgroundPalettes();
    PaletteMatrix GetCGBSpritePalettes();
    void SetTraceLogger(TraceLogger* pTraceLogger);

private:
    void ScanLine(int line);
    void RenderBG(int line, int pixel);
    void RenderWindow(int line);
    void RenderSprites(int line);
    void RenderSpritesNoLimit(int line, int spriteHeight, int lineWidth);
    INLINE void RenderSprite(int line, int sprite, int spriteHeight, int lineWidth);
    void UpdateStatRegister();
    INLINE void TraceEvent(u8 event, u8 value);
    void LogTraceEvent(u8 event, u8 value);

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    u16* m_pColorFrameBuffer;
    int* m_pSpriteXCacheBuffer;
    u8* m_pColorCacheBuffer;
    int m_iStatusMode;
    int m_iStatusModeCounter;
    int m_iStatusModeCounterAux;
    int m_iPendingVBlankInterruptCycles;
    int m_iStatusModeLYCounter;
    int m_iScreenEnableDelayCycles;
    int m_iStatusVBlankLine;
    int m_iPixelCounter;
    int m_iTileCycleCounter;
    bool m_bScreenEnabled;
    bool m_bCGB;
    bool m_bSGBTransferMode;
    bool m_bNoSpriteLimit;
    u16 m_CGBSpritePalettes[8][4][2];
    u16 m_CGBBackgroundPalettes[8][4][2];
    bool m_bScanLineTransfered;
    int m_iWindowLine;
    bool m_bWindowYTrigger;
    int m_iHideFrames;
    u8 m_IRQ48Signal;
    GB_Color_Format m_pixelFormat;
    TraceLogger* m_pTraceLogger;
};

INLINE void Video::TraceEvent(u8 event, u8 value)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_LCD, event))
        LogTraceEvent(event, value);
}

#endif	/* VIDEO_H */
