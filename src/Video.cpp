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

#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "TraceLogger.h"

Video::Video(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pMemory->SetVideo(this);
    m_pProcessor = pProcessor;
    InitPointer(m_pFrameBuffer);
    InitPointer(m_pColorFrameBuffer);
    InitPointer(m_pSpriteXCacheBuffer);
    InitPointer(m_pColorCacheBuffer);
    InitPointer(m_pTraceLogger);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iPendingVBlankInterruptCycles = 0;
    m_iStatusModeLYCounter = 0;
    m_iScreenEnableDelayCycles = 0;
    m_iStatusVBlankLine = 0;
    m_iWindowLine = 0;
    m_bWindowYTrigger = false;
    m_iPixelCounter = 0;
    m_iTileCycleCounter = 0;
    m_bScreenEnabled = true;
    m_bCGB = false;
    m_bSGBTransferMode = false;
    m_bNoSpriteLimit = false;
    InitPointer(m_pColorCorrectionLUT);
    m_bColorCorrectionEnabled = false;
    m_CGBWhiteColor = 0xFFFF;
    m_bScanLineTransfered = false;
    m_iHideFrames = 0;
    m_IRQ48Signal = 0;
    m_pixelFormat = GB_PIXEL_RGB565;
}

Video::~Video()
{
    SafeDeleteArray(m_pSpriteXCacheBuffer);
    SafeDeleteArray(m_pColorCacheBuffer);
    SafeDeleteArray(m_pFrameBuffer);
}

void Video::Init()
{
    m_pFrameBuffer = new u8[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    m_pSpriteXCacheBuffer = new int[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    m_pColorCacheBuffer = new u8[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    Reset(false);
}

void Video::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void Video::LogTraceEvent(u8 event, u8 value)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_LCD;
    e.lcd.event = event;
    e.lcd.value = value;
    e.lcd.line = (u16)m_iStatusModeLYCounter;
    e.lcd.mode = (u8)m_iStatusMode;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(value);
#endif
}

void Video::SetSGBTransferMode(bool enabled)
{
    m_bSGBTransferMode = enabled;
}

void Video::SetNoSpriteLimit(bool noSpriteLimit)
{
    m_bNoSpriteLimit = noSpriteLimit;
}

void Video::Reset(bool bCGB)
{
    for (int i = 0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        m_pSpriteXCacheBuffer[i] = m_pFrameBuffer[i] = m_pColorCacheBuffer[i] = 0;

    for (int p = 0; p < 8; p++)
        for (int c = 0; c < 4; c++)
        {
            // CGB boot ROM fades all BG palettes to white
            m_CGBBackgroundPalettes[p][c][0] = bCGB ? 0x7FFF : 0x0000;
            m_CGBBackgroundPalettes[p][c][1] = bCGB ? 0xFFFF : 0x0000;
            m_CGBSpritePalettes[p][c][0] = 0x0000;
            m_CGBSpritePalettes[p][c][1] = 0x0000;
        }

    RebuildCGBRenderPalettes();

    m_iStatusMode = 1;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iPendingVBlankInterruptCycles = 0;
    m_iStatusModeLYCounter = 144;
    m_iScreenEnableDelayCycles = 0;
    m_iStatusVBlankLine = 0;
    m_iWindowLine = 0;
    m_bWindowYTrigger = false;
    m_iPixelCounter = 0;
    m_iTileCycleCounter = 0;
    m_bScreenEnabled = true;
    m_bScanLineTransfered = false;
    m_bCGB = bCGB;
    m_iHideFrames = 0;
    m_IRQ48Signal = 0;
}

void Video::ResetToBootromState()
{
    m_bScreenEnabled = false;
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 0;
    m_iScreenEnableDelayCycles = 0;
}

void Video::EnableScreen()
{
    if (!m_bScreenEnabled)
    {
        m_iScreenEnableDelayCycles = 244;
    }
}

void Video::DisableScreen()
{
    bool disabled_in_vblank = m_bCGB && m_bScreenEnabled && (m_iStatusMode == 1);

    m_bScreenEnabled = false;
    m_pMemory->Load(0xFF44, 0x00);
    u8 stat = m_pMemory->Retrieve(0xFF41);
    stat &= 0x7C;
    m_pMemory->Load(0xFF41, stat);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iPendingVBlankInterruptCycles = 0;
    m_iStatusModeLYCounter = 0;
    m_IRQ48Signal = 0;
    m_iHideFrames = disabled_in_vblank ? -1 : 0;

    if (!disabled_in_vblank && IsValidPointer(m_pColorFrameBuffer))
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
}

void Video::SetColorCorrection(const u16* pColorCorrectionLUT, bool enabled)
{
    m_pColorCorrectionLUT = pColorCorrectionLUT;
    m_bColorCorrectionEnabled = enabled;
    RebuildCGBRenderPalettes();
}

void Video::RebuildCGBRenderPalettes()
{
    for (int palette = 0; palette < 8; palette++)
    {
        for (int color = 0; color < 4; color++)
        {
            UpdateCGBRenderPalette(true, palette, color);
            UpdateCGBRenderPalette(false, palette, color);
        }
    }

    m_CGBWhiteColor = (m_bColorCorrectionEnabled &&
            IsValidPointer(m_pColorCorrectionLUT)) ?
            m_pColorCorrectionLUT[0xFFFF] : 0xFFFF;
}

void Video::UpdateCGBRenderPalette(bool background, int palette, int color)
{
    u16 normal = background ? m_CGBBackgroundPalettes[palette][color][1] :
            m_CGBSpritePalettes[palette][color][1];
    u16 render = (m_bColorCorrectionEnabled &&
            IsValidPointer(m_pColorCorrectionLUT)) ?
            m_pColorCorrectionLUT[normal] : normal;

    if (background)
        m_CGBBackgroundRenderPalettes[palette][color] = render;
    else
        m_CGBSpriteRenderPalettes[palette][color] = render;
}

void Video::UpdatePaletteToSpecification(bool background, u8 value)
{
    bool hl = IsSetBit(value, 0);
    int index = (value >> 1) & 0x03;
    int pal = (value >> 3) & 0x07;

    u16 color = (background ? m_CGBBackgroundPalettes[pal][index][0] : m_CGBSpritePalettes[pal][index][0]);

    m_pMemory->Load(background ? 0xFF69 : 0xFF6B, hl ? (color >> 8) & 0xFF : color & 0xFF);
}

void Video::SetColorPalette(bool background, u8 value)
{
    u8 ps = background ? m_pMemory->Retrieve(0xFF68) : m_pMemory->Retrieve(0xFF6A);
    bool hl = IsSetBit(ps, 0);
    int index = (ps >> 1) & 0x03;
    int pal = (ps >> 3) & 0x07;
    bool increment = IsSetBit(ps, 7);

    if (increment)
    {
        u8 address = ps & 0x3F;
        address++;
        address &= 0x3F;
        ps = (ps & 0x80) | address;
        m_pMemory->Load(background ? 0xFF68 : 0xFF6A, ps);
        UpdatePaletteToSpecification(background, ps);
    }

    u16* palette_color_gbc = background ? &m_CGBBackgroundPalettes[pal][index][0] : &m_CGBSpritePalettes[pal][index][0];
    u16* palette_color_final = background ? &m_CGBBackgroundPalettes[pal][index][1] : &m_CGBSpritePalettes[pal][index][1];

    *palette_color_gbc = hl ? (*palette_color_gbc & 0x00FF) | (value << 8) : (*palette_color_gbc & 0xFF00) | value;
    
    u8 red_5bit = *palette_color_gbc & 0x1F;
    u8 blue_5bit = (*palette_color_gbc >> 10) & 0x1F;

    switch (m_pixelFormat)
    {
        case GB_PIXEL_RGB565:
        {
            u8 green_5bit = (*palette_color_gbc >> 5) & 0x1F;
            u8 green_6bit = (green_5bit << 1) | (green_5bit >> 4);
            *palette_color_final = (red_5bit << 11) | (green_6bit << 5) | blue_5bit;
            break;
        }
        case GB_PIXEL_BGR565:
        {
            u8 green_5bit = (*palette_color_gbc >> 5) & 0x1F;
            u8 green_6bit = (green_5bit << 1) | (green_5bit >> 4);
            *palette_color_final = (blue_5bit << 11) | (green_6bit << 5) | red_5bit;
            break;
        }
        case GB_PIXEL_RGB555:
        {
            u8 green_5bit = (*palette_color_gbc >> 5) & 0x1F;
            *palette_color_final = 0x8000 | (red_5bit << 10) | (green_5bit << 5) | blue_5bit;
            break;
        }
        case GB_PIXEL_BGR555:
        {
            u8 green_5bit = (*palette_color_gbc >> 5) & 0x1F;
            *palette_color_final = 0x8000 | (blue_5bit << 10) | (green_5bit << 5) | red_5bit;
            break;
        }
    }

    UpdateCGBRenderPalette(background, pal, index);

}

void Video::RefreshStatInterruptSignal(bool requestInterrupt)
{
    u8 signal = 0;

    if (m_bScreenEnabled)
    {
        u8 stat = m_pMemory->Retrieve(0xFF41);
        if (IsSetBit(stat, 3) && (m_iStatusMode == 0))
            signal = SetBit(signal, 0);
        if (IsSetBit(stat, 4) && (m_iStatusMode == 1))
            signal = SetBit(signal, 1);
        if (IsSetBit(stat, 5) && (m_iStatusMode == 2))
            signal = SetBit(signal, 2);
        if (IsSetBit(stat, 6) && (m_pMemory->Retrieve(0xFF45) == m_iStatusModeLYCounter))
            signal = SetBit(signal, 3);
    }

    if (requestInterrupt && (m_IRQ48Signal == 0) && (signal != 0))
    {
        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
        TraceEvent(TRACE_LCD_STAT_IRQ, signal);
    }

    m_IRQ48Signal = signal;
}

void Video::CheckWindowY()
{
    if (m_bWindowYTrigger)
        return;

    u8 lcdc = m_pMemory->Retrieve(0xFF40);
    if (!IsSetBit(lcdc, 5))
        return;

    u8 wy = m_pMemory->Retrieve(0xFF4A);
    if (wy == (u8)m_iStatusModeLYCounter)
        m_bWindowYTrigger = true;
}

void Video::ResetWindowLine()
{
    if ((m_iWindowLine == 0) && (m_iStatusModeLYCounter < 144))
    {
        u8 wy = m_pMemory->Retrieve(0xFF4A);

        if ((m_iStatusModeLYCounter == wy) && (m_iStatusMode == 3) && !m_bScanLineTransfered)
            m_iWindowLine = -1;
    }

    CheckWindowY();
}

void Video::ScanLine(int line)
{
    if (m_iHideFrames > 0 && !m_bSGBTransferMode)
        return;

    if (IsValidPointer(m_pColorFrameBuffer))
    {
        u8 lcdc = m_pMemory->Retrieve(0xFF40);

        if (m_bScreenEnabled && IsSetBit(lcdc, 7))
        {
#ifdef PERFORMANCE
            RenderBG(line, 0);
#endif
            RenderWindow(line);
            RenderSprites(line);
        }
        else
        {
            int line_width = (line * GAMEBOY_WIDTH);
            if (m_bCGB)
            {
                for (int x = 0; x < GAMEBOY_WIDTH; x++)
                    m_pColorFrameBuffer[line_width + x] = m_CGBWhiteColor;
            }
            else
            {
                for (int x = 0; x < GAMEBOY_WIDTH; x++)
                    m_pFrameBuffer[line_width + x] = 0;
            }
        }
    }
}

void Video::RenderBG(int line, int pixel)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);
    int line_width = (line * GAMEBOY_WIDTH);
    
    if (m_bCGB || IsSetBit(lcdc, 0))
    {
#ifdef PERFORMANCE
        int pixels_to_render = 160;
#else
        int pixels_to_render = 4;
#endif
        int tile_start_addr = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map_start_addr = IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800;
        u8 scroll_x = m_pMemory->Retrieve(0xFF43);
        u8 scroll_y = m_pMemory->Retrieve(0xFF42);
        u8 line_scrolled = line + scroll_y;
        int line_scrolled_32 = (line_scrolled >> 3) << 5;
        int tile_pixel_y = line_scrolled & 0x7;
        int tile_pixel_y_2 = tile_pixel_y << 1;
        int tile_pixel_y_flip_2 = (7 - tile_pixel_y) << 1;
        u8 palette = m_pMemory->Retrieve(0xFF47);
        int screen_pixel_x = pixel;
        int remaining = pixels_to_render;

        while (remaining > 0)
        {
            u8 map_pixel_x = screen_pixel_x + scroll_x;
            int map_tile_x = map_pixel_x >> 3;
            int map_tile_offset_x = map_pixel_x & 0x7;
            int run = 8 - map_tile_offset_x;
            if (run > remaining)
                run = remaining;

            u16 map_tile_addr = map_start_addr + line_scrolled_32 + map_tile_x;
            int map_tile = 0;

            if (tile_start_addr == 0x8800)
            {
                map_tile = (s8)m_pMemory->Retrieve(map_tile_addr);
                map_tile += 128;
            }
            else
            {
                map_tile = m_pMemory->Retrieve(map_tile_addr);
            }

            int map_tile_16 = map_tile << 4;

            if (m_bCGB)
            {
                u8 cgb_tile_attr = m_pMemory->ReadCGBLCDRAM(map_tile_addr, true);
                u8 cgb_tile_pal = cgb_tile_attr & 0x07;
                bool cgb_tile_bank = IsSetBit(cgb_tile_attr, 3);
                bool cgb_tile_xflip = IsSetBit(cgb_tile_attr, 5);
                bool cgb_tile_yflip = IsSetBit(cgb_tile_attr, 6);
                bool cgb_tile_priority = IsSetBit(cgb_tile_attr, 7) &&
                        IsSetBit(lcdc, 0);
                int final_pixely_2 = cgb_tile_yflip ?
                        tile_pixel_y_flip_2 : tile_pixel_y_2;
                int tile_address = tile_start_addr + map_tile_16 + final_pixely_2;
                u8 byte1 = 0;
                u8 byte2 = 0;

                if (cgb_tile_bank)
                {
                    byte1 = m_pMemory->ReadCGBLCDRAM(tile_address, true);
                    byte2 = m_pMemory->ReadCGBLCDRAM(tile_address + 1, true);
                }
                else
                {
                    byte1 = m_pMemory->Retrieve(tile_address);
                    byte2 = m_pMemory->Retrieve(tile_address + 1);
                }

                int pixel_bit = cgb_tile_xflip ?
                        (0x01 << map_tile_offset_x) :
                        (0x80 >> map_tile_offset_x);

                for (int i = 0; i < run; i++)
                {
                    int pixel_data = (byte1 & pixel_bit) ? 1 : 0;
                    pixel_data |= (byte2 & pixel_bit) ? 2 : 0;

                    int index = line_width + screen_pixel_x + i;
                    m_pColorCacheBuffer[index] = pixel_data & 0x03;
                    if (cgb_tile_priority && (pixel_data != 0))
                        m_pColorCacheBuffer[index] =
                                SetBit(m_pColorCacheBuffer[index], 2);
                    m_pColorFrameBuffer[index] =
                            m_CGBBackgroundRenderPalettes[cgb_tile_pal][pixel_data];

                    pixel_bit = cgb_tile_xflip ?
                            (pixel_bit << 1) : (pixel_bit >> 1);
                }
            }
            else
            {
                int tile_address = tile_start_addr + map_tile_16 + tile_pixel_y_2;
                u8 byte1 = m_pMemory->Retrieve(tile_address);
                u8 byte2 = m_pMemory->Retrieve(tile_address + 1);
                int pixel_bit = 0x80 >> map_tile_offset_x;

                for (int i = 0; i < run; i++)
                {
                    int pixel_data = (byte1 & pixel_bit) ? 1 : 0;
                    pixel_data |= (byte2 & pixel_bit) ? 2 : 0;

                    int index = line_width + screen_pixel_x + i;
                    m_pColorCacheBuffer[index] = pixel_data & 0x03;
                    m_pFrameBuffer[index] =
                            (palette >> (pixel_data << 1)) & 0x03;

                    pixel_bit >>= 1;
                }
            }

            screen_pixel_x += run;
            remaining -= run;
        }
    }
    else
    {
#ifdef PERFORMANCE
        int pixels_to_clear = 160;
#else
        int pixels_to_clear = 4;
#endif
        for (int x = 0; x < pixels_to_clear; x++)
        {
            int position = line_width + pixel + x;
            m_pFrameBuffer[position] = 0;
            m_pColorCacheBuffer[position] = 0;
        }
    }
}

void Video::RenderWindow(int line)
{
    if (m_iWindowLine > 143)
        return;

    if (m_iWindowLine < 0)
    {
        m_iWindowLine = 0;
        return;
    }

    u8 lcdc = m_pMemory->Retrieve(0xFF40);
    if (!IsSetBit(lcdc, 5))
        return;

    if (!m_bWindowYTrigger)
        return;

    int wx = m_pMemory->Retrieve(0xFF4B) - 7;
    if (wx > 159)
        return;

    u8 wy = m_pMemory->Retrieve(0xFF4A);
    if ((wy > 143) || (wy > line))
        return;

    int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
    int map = IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800;
    int lineAdjusted = m_iWindowLine;
    int y_32 = (lineAdjusted >> 3) << 5;
    int pixely = lineAdjusted & 0x7;
    int pixely_2 = pixely << 1;
    int pixely_2_flip = (7 - pixely) << 1;
    int line_width = (line * GAMEBOY_WIDTH);
    u8 palette = m_pMemory->Retrieve(0xFF47);
    int last_tile = (GAMEBOY_WIDTH - 1 - wx) >> 3;

    for (int x = 0; x <= last_tile; x++)
    {
        int tile = 0;

        if (tiles == 0x8800)
        {
            tile = static_cast<s8> (m_pMemory->Retrieve(map + y_32 + x));
            tile += 128;
        }
        else
        {
            tile = m_pMemory->Retrieve(map + y_32 + x);
        }

        u8 cgb_tile_attr = m_bCGB ? m_pMemory->ReadCGBLCDRAM(map + y_32 + x, true) : 0;
        u8 cgb_tile_pal = m_bCGB ? (cgb_tile_attr & 0x07) : 0;
        bool cgb_tile_bank = m_bCGB ? IsSetBit(cgb_tile_attr, 3) : false;
        bool cgb_tile_xflip = m_bCGB ? IsSetBit(cgb_tile_attr, 5) : false;
        bool cgb_tile_yflip = m_bCGB ? IsSetBit(cgb_tile_attr, 6) : false;
        int mapOffsetX = x << 3;
        int tile_16 = tile << 4;
        u8 byte1 = 0;
        u8 byte2 = 0;
        int final_pixely_2 = (m_bCGB && cgb_tile_yflip) ? pixely_2_flip : pixely_2;
        int tile_address = tiles + tile_16 + final_pixely_2;

        if (m_bCGB && cgb_tile_bank)
        {
            byte1 = m_pMemory->ReadCGBLCDRAM(tile_address, true);
            byte2 = m_pMemory->ReadCGBLCDRAM(tile_address + 1, true);
        }
        else
        {
            byte1 = m_pMemory->Retrieve(tile_address);
            byte2 = m_pMemory->Retrieve(tile_address + 1);
        }

        for (int pixelx = 0; pixelx < 8; pixelx++)
        {
            int bufferX = (mapOffsetX + pixelx + wx);

            if (bufferX < 0 || bufferX >= GAMEBOY_WIDTH)
                continue;

            int pixelx_pos = pixelx;

            if (m_bCGB && cgb_tile_xflip)
            {
                pixelx_pos = 7 - pixelx_pos;
            }

            int pixel = (byte1 & (0x1 << (7 - pixelx_pos))) ? 1 : 0;
            pixel |= (byte2 & (0x1 << (7 - pixelx_pos))) ? 2 : 0;

            int position = line_width + bufferX;
            m_pColorCacheBuffer[position] = pixel & 0x03;

            if (m_bCGB)
            {
                bool cgb_tile_priority = IsSetBit(cgb_tile_attr, 7) && IsSetBit(lcdc, 0);
                if (cgb_tile_priority && (pixel != 0))
                    m_pColorCacheBuffer[position] = SetBit(m_pColorCacheBuffer[position], 2);
                m_pColorFrameBuffer[position] =
                        m_CGBBackgroundRenderPalettes[cgb_tile_pal][pixel];
            }
            else
            {
                u8 color = (palette >> (pixel << 1)) & 0x03;
                m_pFrameBuffer[position] = color;
            }
        }
    }
    m_iWindowLine++;
}

INLINE void Video::RenderSprite(int line, int sprite, int sprite_height, int line_width)
{
    int sprite_4 = sprite << 2;
    int sprite_x = m_pMemory->Retrieve(0xFE00 + sprite_4 + 1) - 8;

    if ((sprite_x < -7) || (sprite_x >= GAMEBOY_WIDTH))
        return;

    int sprite_y = m_pMemory->Retrieve(0xFE00 + sprite_4) - 16;
    int sprite_tile_16 = (m_pMemory->Retrieve(0xFE00 + sprite_4 + 2)
            & ((sprite_height == 16) ? 0xFE : 0xFF)) << 4;
    u8 sprite_flags = m_pMemory->Retrieve(0xFE00 + sprite_4 + 3);
    int sprite_pallette = IsSetBit(sprite_flags, 4) ? 1 : 0;
    u8 palette = m_pMemory->Retrieve(sprite_pallette ? 0xFF49 : 0xFF48);
    bool xflip = IsSetBit(sprite_flags, 5);
    bool yflip = IsSetBit(sprite_flags, 6);
    bool aboveBG = (!IsSetBit(sprite_flags, 7));
    bool cgb_tile_bank = IsSetBit(sprite_flags, 3);
    int cgb_tile_pal = sprite_flags & 0x07;
    int tiles = 0x8000;
    int pixel_y = yflip ? ((sprite_height == 16) ? 15 : 7) - (line - sprite_y) : line - sprite_y;
    u8 byte1 = 0;
    u8 byte2 = 0;
    int pixel_y_2 = 0;
    int offset = 0;

    if (sprite_height == 16 && (pixel_y >= 8))
    {
        pixel_y_2 = (pixel_y - 8) << 1;
        offset = 16;
    }
    else
        pixel_y_2 = pixel_y << 1;

    int tile_address = tiles + sprite_tile_16 + pixel_y_2 + offset;

    if (m_bCGB && cgb_tile_bank)
    {
        byte1 = m_pMemory->ReadCGBLCDRAM(tile_address, true);
        byte2 = m_pMemory->ReadCGBLCDRAM(tile_address + 1, true);
    }
    else
    {
        byte1 = m_pMemory->Retrieve(tile_address);
        byte2 = m_pMemory->Retrieve(tile_address + 1);
    }

    for (int pixelx = 0; pixelx < 8; pixelx++)
    {
        int pixel = (byte1 & (0x01 << (xflip ? pixelx : 7 - pixelx))) ? 1 : 0;
        pixel |= (byte2 & (0x01 << (xflip ? pixelx : 7 - pixelx))) ? 2 : 0;

        if (pixel == 0)
            continue;

        int bufferX = (sprite_x + pixelx);

        if (bufferX < 0 || bufferX >= GAMEBOY_WIDTH)
            continue;

        int position = line_width + bufferX;
        u8 color_cache = m_pColorCacheBuffer[position];

        if (m_bCGB)
        {
            if (IsSetBit(color_cache, 2))
                continue;
        }
        else
        {
            int sprite_x_cache = m_pSpriteXCacheBuffer[position];
            if (IsSetBit(color_cache, 3) && (sprite_x_cache < sprite_x))
                continue;
        }

        if (!aboveBG && (color_cache & 0x03))
            continue;

        m_pColorCacheBuffer[position] = SetBit(color_cache, 3);
        m_pSpriteXCacheBuffer[position] = sprite_x;
        if (m_bCGB)
        {
            m_pColorFrameBuffer[position] =
                    m_CGBSpriteRenderPalettes[cgb_tile_pal][pixel];
        }
        else
        {
            u8 color = (palette >> (pixel << 1)) & 0x03;
            m_pFrameBuffer[position] = color;
        }
    }
}

void Video::RenderSprites(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (!IsSetBit(lcdc, 1))
        return;

    int sprite_height = IsSetBit(lcdc, 2) ? 16 : 8;
    int line_width = (line * GAMEBOY_WIDTH);

    if (unlikely(m_bNoSpriteLimit))
    {
        RenderSpritesNoLimit(line, sprite_height, line_width);
        return;
    }

    u8 visible_sprites[10];
    int visible_count = 0;

    for (int sprite = 0; sprite < 40; sprite++)
    {
        int sprite_4 = sprite << 2;
        int sprite_y = m_pMemory->Retrieve(0xFE00 + sprite_4) - 16;

        if ((sprite_y > line) || ((sprite_y + sprite_height) <= line))
            continue;

        visible_sprites[visible_count++] = (u8)sprite;
        if (visible_count == 10)
            break;
    }

    for (int selected = visible_count - 1; selected >= 0; selected--)
        RenderSprite(line, visible_sprites[selected], sprite_height, line_width);
}

void Video::RenderSpritesNoLimit(int line, int sprite_height, int line_width)
{
    for (int sprite = 39; sprite >= 0; sprite--)
    {
        int sprite_y = m_pMemory->Retrieve(0xFE00 + (sprite << 2)) - 16;

        if ((sprite_y > line) || ((sprite_y + sprite_height) <= line))
            continue;

        RenderSprite(line, sprite, sprite_height, line_width);
    }
}

void Video::UpdateStatRegister()
{
    // Updates the STAT register with current mode
    u8 stat = m_pMemory->Retrieve(0xFF41);
    m_pMemory->Load(0xFF41, (stat & 0xFC) | (m_iStatusMode & 0x3));
}

void Video::CompareLYToLYC()
{
    if (m_bScreenEnabled)
    {
        u8 lyc = m_pMemory->Retrieve(0xFF45);
        u8 stat = m_pMemory->Retrieve(0xFF41);

        if (lyc == m_iStatusModeLYCounter)
        {
            stat = SetBit(stat, 2);
        }
        else
        {
            stat = UnsetBit(stat, 2);
        }

        m_pMemory->Load(0xFF41, stat);
        RefreshStatInterruptSignal(true);
    }
}

void Video::SetIRQ48Signal(u8 signal)
{
    m_IRQ48Signal = signal;
}

void Video::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_pFrameBuffer), GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.write(reinterpret_cast<const char*> (m_pSpriteXCacheBuffer), sizeof(int) * GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.write(reinterpret_cast<const char*> (m_pColorCacheBuffer), GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.write(reinterpret_cast<const char*> (&m_iStatusMode), sizeof(m_iStatusMode));
    stream.write(reinterpret_cast<const char*> (&m_iStatusModeCounter), sizeof(m_iStatusModeCounter));
    stream.write(reinterpret_cast<const char*> (&m_iStatusModeCounterAux), sizeof(m_iStatusModeCounterAux));
    stream.write(reinterpret_cast<const char*> (&m_iStatusModeLYCounter), sizeof(m_iStatusModeLYCounter));
    stream.write(reinterpret_cast<const char*> (&m_iScreenEnableDelayCycles), sizeof(m_iScreenEnableDelayCycles));
    stream.write(reinterpret_cast<const char*> (&m_iStatusVBlankLine), sizeof(m_iStatusVBlankLine));
    stream.write(reinterpret_cast<const char*> (&m_iPixelCounter), sizeof(m_iPixelCounter));
    stream.write(reinterpret_cast<const char*> (&m_iTileCycleCounter), sizeof(m_iTileCycleCounter));
    stream.write(reinterpret_cast<const char*> (&m_bScreenEnabled), sizeof(m_bScreenEnabled));
    stream.write(reinterpret_cast<const char*> (m_CGBSpritePalettes), sizeof(m_CGBSpritePalettes));
    stream.write(reinterpret_cast<const char*> (m_CGBBackgroundPalettes), sizeof(m_CGBBackgroundPalettes));
    stream.write(reinterpret_cast<const char*> (&m_bScanLineTransfered), sizeof(m_bScanLineTransfered));
    stream.write(reinterpret_cast<const char*> (&m_iWindowLine), sizeof(m_iWindowLine));
    stream.write(reinterpret_cast<const char*> (&m_iHideFrames), sizeof(m_iHideFrames));
    stream.write(reinterpret_cast<const char*> (&m_IRQ48Signal), sizeof(m_IRQ48Signal));
    stream.write(reinterpret_cast<const char*> (&m_iPendingVBlankInterruptCycles), sizeof(m_iPendingVBlankInterruptCycles));
    stream.write(reinterpret_cast<const char*> (&m_bWindowYTrigger), sizeof(m_bWindowYTrigger));
}

void Video::LoadState(std::istream& stream, u32 version)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pFrameBuffer), GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.read(reinterpret_cast<char*> (m_pSpriteXCacheBuffer), sizeof(int) * GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.read(reinterpret_cast<char*> (m_pColorCacheBuffer), GAMEBOY_WIDTH * GAMEBOY_HEIGHT);
    stream.read(reinterpret_cast<char*> (&m_iStatusMode), sizeof(m_iStatusMode));
    stream.read(reinterpret_cast<char*> (&m_iStatusModeCounter), sizeof(m_iStatusModeCounter));
    stream.read(reinterpret_cast<char*> (&m_iStatusModeCounterAux), sizeof(m_iStatusModeCounterAux));
    stream.read(reinterpret_cast<char*> (&m_iStatusModeLYCounter), sizeof(m_iStatusModeLYCounter));
    stream.read(reinterpret_cast<char*> (&m_iScreenEnableDelayCycles), sizeof(m_iScreenEnableDelayCycles));
    stream.read(reinterpret_cast<char*> (&m_iStatusVBlankLine), sizeof(m_iStatusVBlankLine));
    stream.read(reinterpret_cast<char*> (&m_iPixelCounter), sizeof(m_iPixelCounter));
    stream.read(reinterpret_cast<char*> (&m_iTileCycleCounter), sizeof(m_iTileCycleCounter));
    stream.read(reinterpret_cast<char*> (&m_bScreenEnabled), sizeof(m_bScreenEnabled));
    stream.read(reinterpret_cast<char*> (m_CGBSpritePalettes), sizeof(m_CGBSpritePalettes));
    stream.read(reinterpret_cast<char*> (m_CGBBackgroundPalettes), sizeof(m_CGBBackgroundPalettes));
    stream.read(reinterpret_cast<char*> (&m_bScanLineTransfered), sizeof(m_bScanLineTransfered));
    stream.read(reinterpret_cast<char*> (&m_iWindowLine), sizeof(m_iWindowLine));
    stream.read(reinterpret_cast<char*> (&m_iHideFrames), sizeof(m_iHideFrames));
    stream.read(reinterpret_cast<char*> (&m_IRQ48Signal), sizeof(m_IRQ48Signal));

    if (version >= 101)
    {
        stream.read(reinterpret_cast<char*> (&m_iPendingVBlankInterruptCycles), sizeof(m_iPendingVBlankInterruptCycles));
    }
    else
    {
        m_iPendingVBlankInterruptCycles = 0;
    }

    if (version >= 102)
    {
        stream.read(reinterpret_cast<char*> (&m_bWindowYTrigger), sizeof(m_bWindowYTrigger));
    }
    else
    {
        m_bWindowYTrigger = false;
    }

    RebuildCGBRenderPalettes();
}

PaletteMatrix Video::GetCGBBackgroundPalettes()
{
    return &m_CGBBackgroundPalettes;
}

PaletteMatrix Video::GetCGBSpritePalettes()
{
    return &m_CGBSpritePalettes;
}
