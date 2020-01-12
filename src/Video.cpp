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

Video::Video(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pMemory->SetVideo(this);
    m_pProcessor = pProcessor;
    InitPointer(m_pFrameBuffer);
    InitPointer(m_pColorFrameBuffer);
    InitPointer(m_pSpriteXCacheBuffer);
    InitPointer(m_pColorCacheBuffer);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 0;
    m_iScreenEnableDelayCycles = 0;
    m_iStatusVBlankLine = 0;
    m_iWindowLine = 0;
    m_iPixelCounter = 0;
    m_iTileCycleCounter = 0;
    m_bScreenEnabled = true;
    m_bCGB = false;
    m_bScanLineTransfered = false;
    m_iHideFrames = 0;
    m_IRQ48Signal = 0;
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

void Video::Reset(bool bCGB)
{
    for (int i = 0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        m_pSpriteXCacheBuffer[i] = m_pFrameBuffer[i] = m_pColorCacheBuffer[i] = 0;

    for (int p = 0; p < 8; p++)
        for (int c = 0; c < 4; c++)
            m_CGBBackgroundPalettes[p][c].red = m_CGBBackgroundPalettes[p][c].green =
                m_CGBBackgroundPalettes[p][c].blue = m_CGBSpritePalettes[p][c].red =
                m_CGBSpritePalettes[p][c].green = m_CGBSpritePalettes[p][c].blue = 0;

    m_iStatusMode = 1;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 144;
    m_iScreenEnableDelayCycles = 0;
    m_iStatusVBlankLine = 0;
    m_iWindowLine = 0;
    m_iPixelCounter = 0;
    m_iTileCycleCounter = 0;
    m_bScreenEnabled = true;
    m_bScanLineTransfered = false;
    m_bCGB = bCGB;
    m_iHideFrames = 0;
    m_IRQ48Signal = 0;
}

bool Video::Tick(unsigned int &clockCycles, GB_Color* pColorFrameBuffer)
{
    m_pColorFrameBuffer = pColorFrameBuffer;

    bool vblank = false;
    m_iStatusModeCounter += clockCycles;

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

                    m_iStatusModeLYCounter++;
                    m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                    CompareLYToLYC();

                    if (m_bCGB && m_pMemory->IsHDMAEnabled() && (!m_pProcessor->Halted() || m_pProcessor->InterruptIsAboutToRaise()))
                    {
                        unsigned int cycles = m_pMemory->PerformHDMA();
                        m_iStatusModeCounter += cycles;
                        clockCycles += cycles;
                    }

                    if (m_iStatusModeLYCounter == 144)
                    {
                        m_iStatusMode = 1;
                        m_iStatusVBlankLine = 0;
                        m_iStatusModeCounterAux = m_iStatusModeCounter;

                        m_pProcessor->RequestInterrupt(Processor::VBlank_Interrupt);

                        m_IRQ48Signal &= 0x09;
                        u8 stat = m_pMemory->Retrieve(0xFF41);
                        if (IsSetBit(stat, 4))
                        {
                            if (!IsSetBit(m_IRQ48Signal, 0) && !IsSetBit(m_IRQ48Signal, 3))
                            {
                                m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                            }
                            m_IRQ48Signal = SetBit(m_IRQ48Signal, 1);
                        }
                        m_IRQ48Signal &= 0x0E;

                        if (m_iHideFrames > 0)
                            m_iHideFrames--;
                        else
                            vblank = true;

                        m_iWindowLine = 0;
                    }
                    else
                    {
                        m_IRQ48Signal &= 0x09;
                        u8 stat = m_pMemory->Retrieve(0xFF41);
                        if (IsSetBit(stat, 5))
                        {
                            if (m_IRQ48Signal == 0)
                            {
                                m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                            }
                            m_IRQ48Signal = SetBit(m_IRQ48Signal, 2);
                        }
                        m_IRQ48Signal &= 0x0E;
                    }

                    UpdateStatRegister();
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
                    UpdateStatRegister();
                    m_IRQ48Signal &= 0x07;


                    m_IRQ48Signal &= 0x0A;
                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 5))
                    {
                        if (m_IRQ48Signal == 0)
                        {
                            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                        }
                        m_IRQ48Signal = SetBit(m_IRQ48Signal, 2);
                    }
                    m_IRQ48Signal &= 0x0D;
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
                    m_IRQ48Signal &= 0x08;
                    UpdateStatRegister();
                }
                break;
            }
            // During transfering data to LCD driver
            case 3:
            {
                if (m_iPixelCounter < 160)
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
                    UpdateStatRegister();

                    m_IRQ48Signal &= 0x08;
                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 3))
                    {
                        if (!IsSetBit(m_IRQ48Signal, 3))
                        {
                            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                        }
                        m_IRQ48Signal = SetBit(m_IRQ48Signal, 0);
                    }
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
                m_iHideFrames = 3;
                m_iStatusMode = 0;
                m_iStatusModeCounter = 0;
                m_iStatusModeCounterAux = 0;
                m_iStatusModeLYCounter = 0;
                m_iWindowLine = 0;
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
                }

                CompareLYToLYC();
            }
        }
        else if (m_iStatusModeCounter >= 70224)
        {
            m_iStatusModeCounter -= 70224;
            vblank = true;
        }
    }
    return vblank;
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
    m_bScreenEnabled = false;
    m_pMemory->Load(0xFF44, 0x00);
    u8 stat = m_pMemory->Retrieve(0xFF41);
    stat &= 0x7C;
    m_pMemory->Load(0xFF41, stat);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 0;
    m_IRQ48Signal = 0;
}

bool Video::IsScreenEnabled() const
{
    return m_bScreenEnabled;
}

const u8* Video::GetFrameBuffer() const
{
    return m_pFrameBuffer;
}

void Video::UpdatePaletteToSpecification(bool background, u8 value)
{
    bool hl = IsSetBit(value, 0);
    int index = (value >> 1) & 0x03;
    int pal = (value >> 3) & 0x07;

    GB_Color color = (background ? m_CGBBackgroundPalettes[pal][index] : m_CGBSpritePalettes[pal][index]);

    u8 final_value = 0;

    if (hl)
    {
        u8 blue = (color.blue & 0x1f) << 2;
        u8 half_green_hi = (color.green >> 3) & 0x03;
        final_value = (blue | half_green_hi) & 0x7F;
    }
    else
    {
        u8 half_green_low = (color.green & 0x07) << 5;
        u8 red = color.red & 0x1F;
        final_value = (red | half_green_low);
    }

    m_pMemory->Load(background ? 0xFF69 : 0xFF6B, final_value);
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

    if (hl)
    {
        // high
        u8 blue = (value >> 2) & 0x1F;
        u8 half_green_hi = (value & 0x03) << 3;

        if (background)
        {
            m_CGBBackgroundPalettes[pal][index].blue = blue;
            m_CGBBackgroundPalettes[pal][index].green =
                    (m_CGBBackgroundPalettes[pal][index].green & 0x07) | half_green_hi;
        }
        else
        {
            m_CGBSpritePalettes[pal][index].blue = blue;
            m_CGBSpritePalettes[pal][index].green =
                    (m_CGBSpritePalettes[pal][index].green & 0x07) | half_green_hi;
        }
    }
    else
    {
        // low
        u8 half_green_low = (value >> 5) & 0x07;
        u8 red = value & 0x1F;

        if (background)
        {
            m_CGBBackgroundPalettes[pal][index].red = red;
            m_CGBBackgroundPalettes[pal][index].green =
                    (m_CGBBackgroundPalettes[pal][index].green & 0x18) | half_green_low;
        }
        else
        {
            m_CGBSpritePalettes[pal][index].red = red;
            m_CGBSpritePalettes[pal][index].green =
                    (m_CGBSpritePalettes[pal][index].green & 0x18) | half_green_low;
        }
    }
}

int Video::GetCurrentStatusMode() const
{
    return m_iStatusMode;
}

void Video::ResetWindowLine()
{
    u8 wy = m_pMemory->Retrieve(0xFF4A);

    if ((m_iWindowLine == 0) && (m_iStatusModeLYCounter < 144) && (m_iStatusModeLYCounter > wy))
        m_iWindowLine = 144;
}

void Video::ScanLine(int line)
{
    if (IsValidPointer(m_pColorFrameBuffer))
    {
        u8 lcdc = m_pMemory->Retrieve(0xFF40);

        if (m_bScreenEnabled && IsSetBit(lcdc, 7))
        {
            RenderWindow(line);
            RenderSprites(line);
        }
        else
        {
            int line_width = (line * GAMEBOY_WIDTH);
            if (m_bCGB)
            {
                GB_Color black;
                black.red = 0;
                black.green = 0;
                black.blue = 0;
                black.alpha = 0xFF;
                for (int x = 0; x < GAMEBOY_WIDTH; x++)
                    m_pColorFrameBuffer[line_width + x] = black;
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
        int offset_x_init = pixel & 0x7;
        int offset_x_end = offset_x_init + 4;
        int screen_tile = pixel >> 3;
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

        for (int offset_x = offset_x_init; offset_x < offset_x_end; offset_x++)
        {
            int screen_pixel_x = (screen_tile << 3) + offset_x;
            u8 map_pixel_x = screen_pixel_x + scroll_x;
            int map_tile_x = map_pixel_x >> 3;
            int map_tile_offset_x = map_pixel_x & 0x7;
            u16 map_tile_addr = map_start_addr + line_scrolled_32 + map_tile_x;
            int map_tile = 0;

            if (tile_start_addr == 0x8800)
            {
                map_tile = static_cast<s8> (m_pMemory->Retrieve(map_tile_addr));
                map_tile += 128;
            }
            else
            {
                map_tile = m_pMemory->Retrieve(map_tile_addr);
            }

            u8 cgb_tile_attr = m_bCGB ? m_pMemory->ReadCGBLCDRAM(map_tile_addr, true) : 0;
            u8 cgb_tile_pal = m_bCGB ? (cgb_tile_attr & 0x07) : 0;
            bool cgb_tile_bank = m_bCGB ? IsSetBit(cgb_tile_attr, 3) : false;
            bool cgb_tile_xflip = m_bCGB ? IsSetBit(cgb_tile_attr, 5) : false;
            bool cgb_tile_yflip = m_bCGB ? IsSetBit(cgb_tile_attr, 6) : false;
            int map_tile_16 = map_tile << 4;
            u8 byte1 = 0;
            u8 byte2 = 0;
            int final_pixely_2 = cgb_tile_yflip ? tile_pixel_y_flip_2 : tile_pixel_y_2;
            int tile_address = tile_start_addr + map_tile_16 + final_pixely_2;

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

            int pixel_x_in_tile = map_tile_offset_x;

            if (cgb_tile_xflip)
            {
                pixel_x_in_tile = 7 - pixel_x_in_tile;
            }
            int pixel_x_in_tile_bit = 0x1 << (7 - pixel_x_in_tile);
            int pixel_data = (byte1 & pixel_x_in_tile_bit) ? 1 : 0;
            pixel_data |= (byte2 & pixel_x_in_tile_bit) ? 2 : 0;

            int index = line_width + screen_pixel_x;
            m_pColorCacheBuffer[index] = pixel_data & 0x03;

            if (m_bCGB)
            {
                bool cgb_tile_priority = IsSetBit(cgb_tile_attr, 7);
                if (cgb_tile_priority && (pixel_data != 0))
                    m_pColorCacheBuffer[index] = SetBit(m_pColorCacheBuffer[index], 2);
                GB_Color color = m_CGBBackgroundPalettes[cgb_tile_pal][pixel_data];
                m_pColorFrameBuffer[index] = ConvertTo8BitColor(color);
            }
            else
            {
                u8 color = (palette >> (pixel_data << 1)) & 0x03;
                m_pFrameBuffer[index] = color;
            }
        }
    }
    else
    {
        for (int x = 0; x < 4; x++)
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

    u8 lcdc = m_pMemory->Retrieve(0xFF40);
    if (!IsSetBit(lcdc, 5))
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

    for (int x = 0; x < 32; x++)
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
        bool cgb_tile_priority = m_bCGB ? IsSetBit(cgb_tile_attr, 7) : false;
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
                if (cgb_tile_priority && (pixel != 0))
                    m_pColorCacheBuffer[position] = SetBit(m_pColorCacheBuffer[position], 2);
                GB_Color color = m_CGBBackgroundPalettes[cgb_tile_pal][pixel];
                m_pColorFrameBuffer[position] = ConvertTo8BitColor(color);
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

void Video::RenderSprites(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (!IsSetBit(lcdc, 1))
        return;

    int sprite_height = IsSetBit(lcdc, 2) ? 16 : 8;
    int line_width = (line * GAMEBOY_WIDTH);

    for (int sprite = 39; sprite >= 0; sprite--)
    {
        int sprite_4 = sprite << 2;
        int sprite_y = m_pMemory->Retrieve(0xFE00 + sprite_4) - 16;

        if ((sprite_y > line) || ((sprite_y + sprite_height) <= line))
            continue;

        int sprite_x = m_pMemory->Retrieve(0xFE00 + sprite_4 + 1) - 8;

        if ((sprite_x < -7) || (sprite_x >= GAMEBOY_WIDTH))
            continue;

        int sprite_tile_16 = (m_pMemory->Retrieve(0xFE00 + sprite_4 + 2)
                & ((sprite_height == 16) ? 0xFE : 0xFF)) << 4;
        u8 sprite_flags = m_pMemory->Retrieve(0xFE00 + sprite_4 + 3);
        int sprite_pallette = IsSetBit(sprite_flags, 4) ? 1 : 0;
        u8 palette = m_pMemory->Retrieve(sprite_pallette ? 0xFF49 : 0xFF48);
        bool xflip = IsSetBit(sprite_flags, 5);
        bool yflip = IsSetBit(sprite_flags, 6);
        bool aboveBG = !IsSetBit(sprite_flags, 7);
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
                GB_Color color = m_CGBSpritePalettes[cgb_tile_pal][pixel];
                m_pColorFrameBuffer[position] = ConvertTo8BitColor(color);
            }
            else
            {
                u8 color = (palette >> (pixel << 1)) & 0x03;
                m_pFrameBuffer[position] = color;
            }
        }
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
            if (IsSetBit(stat, 6))
            {
                if (m_IRQ48Signal == 0)
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                }
                m_IRQ48Signal = SetBit(m_IRQ48Signal, 3);
            }
        }
        else
        {
            stat = UnsetBit(stat, 2);
            m_IRQ48Signal = UnsetBit(m_IRQ48Signal, 3);
        }

        m_pMemory->Load(0xFF41, stat);
    }
}

u8 Video::GetIRQ48Signal() const
{
    return m_IRQ48Signal;
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
}

void Video::LoadState(std::istream& stream)
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
}
