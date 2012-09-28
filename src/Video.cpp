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
    m_bScreenEnabled = true;
    m_bCGB = false;
    m_bScanLineTransfered = false;
    m_iHideFrames = 0;
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
    m_bScreenEnabled = true;
    m_bScanLineTransfered = false;
    m_bCGB = bCGB;
    m_iHideFrames = 0;
}

bool Video::Tick(u8 clockCycles, GB_Color* pColorFrameBuffer)
{
    m_pColorFrameBuffer = pColorFrameBuffer;

    bool vblank = false;

    if (m_bScreenEnabled)
    {
        m_iStatusModeCounter += clockCycles;

        switch (m_iStatusMode)
        {
            case 0:
            {
                // During H-BLANK
                if (m_iStatusModeCounter >= 204)
                {
                    m_iStatusModeCounter -= 204;
                    m_iStatusMode = 2;

                    m_iStatusModeLYCounter++;
                    m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                    CompareLYToLYC();

                    if (m_bCGB && m_pMemory->IsHBDMAEnabled())
                        m_pMemory->DoHDMACGBTransfer(true);

                    if (m_iStatusModeLYCounter == 144)
                    {
                        m_iStatusMode = 1;
                        m_iStatusVBlankLine = 0;
                        m_iStatusModeCounterAux = m_iStatusModeCounter;

                        m_pProcessor->RequestInterrupt(Processor::VBlank_Interrupt);

                        u8 stat = m_pMemory->Retrieve(0xFF41);
                        if (IsSetBit(stat, 4))
                            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);

                        if (m_iHideFrames > 0)
                            m_iHideFrames--;
                        else
                            vblank = true;

                        m_iWindowLine = 0;
                    }
                    else
                    {
                        u8 stat = m_pMemory->Retrieve(0xFF41);
                        if (IsSetBit(stat, 5))
                            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                    }

                    UpdateStatRegister();
                }
                break;
            }
            case 1:
            {
                // During V-BLANK
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

                if ((m_iStatusModeCounter >= 4104) && (m_iStatusModeCounterAux >= 32) && (m_iStatusModeLYCounter == 153))
                {
                    m_iStatusModeLYCounter = 0;
                    m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                }

                if (m_iStatusModeCounter >= 4560)
                {
                    m_iStatusModeCounter -= 4560;
                    m_iStatusMode = 2;
                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 5))
                        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                    UpdateStatRegister();
                    CompareLYToLYC();
                }
                break;
            }
            case 2:
            {
                // During searching OAM RAM
                if (m_iStatusModeCounter >= 80)
                {
                    m_iStatusModeCounter -= 80;
                    m_iStatusMode = 3;
                    m_bScanLineTransfered = false;
                    UpdateStatRegister();
                }
                break;
            }
            case 3:
            {
                // During transfering data to LCD driver
                if (!m_bScanLineTransfered && (m_iStatusModeCounter >= 48))
                {
                    m_bScanLineTransfered = true;
                    //ScanLine(m_iStatusModeLYCounter);
                }

                if (m_iStatusModeCounter >= 172)
                {
                    ScanLine(m_iStatusModeLYCounter);
                    m_iStatusModeCounter -= 172;
                    m_iStatusMode = 0;

                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 3))
                        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);

                    UpdateStatRegister();
                }
                break;
            }
        }
    }
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
                m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);
                m_pProcessor->SetIRQ48Signal(0);

                u8 stat = m_pMemory->Retrieve(0xFF41);
                if (IsSetBit(stat, 5))
                {
                    u8 signal = m_pProcessor->GetIRQ48Signal();
                    if (signal == 0x00)
                    {
                        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                    }
                    m_pProcessor->SetIRQ48Signal(SetBit(signal, 2));
                }

                CompareLYToLYC();
            }
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
}

bool Video::IsScreenEnabled() const
{
    return m_bScreenEnabled;
}

const u8* Video::GetFrameBuffer() const
{
    return m_pFrameBuffer;
}

void Video::SetColorPalette(bool background, u8 value)
{
    u8 bcps = m_pMemory->Retrieve(0xFF68);
    bool hl = IsSetBit(bcps, 0);
    int index = (bcps >> 1) & 0x03;
    int pal = (bcps >> 3) & 0x07;
    bool increment = IsSetBit(bcps, 7);

    if (increment)
    {
        u8 address = bcps & 0x3F;
        address++;
        address &= 0x3F;
        bcps = (bcps & 0xC0) | address;
        m_pMemory->Load(0xFF68, bcps);
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

    if ((m_iWindowLine == 0) && (m_iStatusModeLYCounter > wy))
        m_iWindowLine = 144;
}

void Video::ScanLine(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (m_bScreenEnabled && IsSetBit(lcdc, 7))
    {
        RenderBG(line);
        RenderWindow(line);
        RenderSprites(line);
    }
    else
    {
        for (int x = 0; x < GAMEBOY_WIDTH; x++)
            m_pFrameBuffer[(line * GAMEBOY_WIDTH) + x] = 0;
    }
}

void Video::RenderBG(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (m_bCGB || IsSetBit(lcdc, 0))
    {
        int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map = IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800;
        u8 scx = m_pMemory->Retrieve(0xFF43);
        u8 scy = m_pMemory->Retrieve(0xFF42);
        u8 lineAdjusted = line + scy;
        int y_32 = (lineAdjusted / 8) * 32;
        int pixely = lineAdjusted % 8;
        int pixely_2 = pixely * 2;

        for (int x = 0; x < 32; x++)
        {
            int tile = 0;

            if (tiles == 0x8800)
            {
                if (m_bCGB)
                    tile = static_cast<s8> (m_pMemory->ReadCGBLCDRAM(map + y_32 + x, false));
                else
                    tile = static_cast<s8> (m_pMemory->Retrieve(map + y_32 + x));
                tile += 128;
            }
            else
            {
                if (m_bCGB)
                    tile = m_pMemory->ReadCGBLCDRAM(map + y_32 + x, false);
                else
                    tile = m_pMemory->Retrieve(map + y_32 + x);
            }

            u8 cgb_tile_attr = 0;
            u8 cgb_tile_pal = 0;
            bool cgb_tile_bank = false;
            bool cgb_tile_yflip = false;
            bool cgb_tile_xflip = false;
            bool cgb_tile_priority = false;
            if (m_bCGB)
            {
                cgb_tile_attr = m_pMemory->ReadCGBLCDRAM(map + y_32 + x, true);
                cgb_tile_pal = cgb_tile_attr & 0x07;
                cgb_tile_bank = IsSetBit(cgb_tile_attr, 3);
                cgb_tile_xflip = IsSetBit(cgb_tile_attr, 5);
                cgb_tile_yflip = IsSetBit(cgb_tile_attr, 6);
                cgb_tile_priority = IsSetBit(cgb_tile_attr, 7);
            }
            int mapOffsetX = x * 8;
            int tile_16 = tile * 16;
            u8 byte1 = 0;
            u8 byte2 = 0;
            /*
            if (m_bCGB && cgb_tile_yflip)
            {
                pixely = 7 - pixely;
                pixely_2 = pixely * 2;
            }
             */
            if (m_bCGB && cgb_tile_bank)
            {
                byte1 = m_pMemory->ReadCGBLCDRAM(tiles + tile_16 + pixely_2, true);
                byte2 = m_pMemory->ReadCGBLCDRAM(tiles + tile_16 + pixely_2 + 1, true);
            }
            else
            {
                byte1 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2);
                byte2 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2 + 1);
            }

            for (int pixelx = 0; pixelx < 8; pixelx++)
            {
                u8 bufferX = (mapOffsetX + pixelx - scx);

                if (bufferX < GAMEBOY_WIDTH)
                {
                    int pixelx_pos = pixelx;

                    if (m_bCGB && cgb_tile_xflip)
                    {
                        pixelx_pos = 7 - pixelx_pos;
                    }

                    int pixel = (byte1 & (0x1 << (7 - pixelx_pos))) ? 1 : 0;
                    pixel |= (byte2 & (0x1 << (7 - pixelx_pos))) ? 2 : 0;

                    int position = (line * GAMEBOY_WIDTH) + bufferX;
                    m_pColorCacheBuffer[position] = pixel & 0x03;
                    if (m_bCGB)
                    {
                        GB_Color color = m_CGBBackgroundPalettes[cgb_tile_pal][pixel];
                        m_pColorFrameBuffer[position] = ConvertTo8BitColor(color);
                    }
                    else
                    {
                        u8 palette = m_pMemory->Retrieve(0xFF47);
                        u8 color = (palette >> (pixel * 2)) & 0x03;
                        m_pFrameBuffer[position] = color;
                    }
                }
            }
        }
    }
    else
    {
        for (int x = 0; x < GAMEBOY_WIDTH; x++)
        {
            m_pFrameBuffer[(line * GAMEBOY_WIDTH) + x] = 0;
            m_pColorCacheBuffer[(line * GAMEBOY_WIDTH) + x] = 0;
        }
    }
}

void Video::RenderWindow(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);
    int wx = m_pMemory->Retrieve(0xFF4B) - 7;

    if (IsSetBit(lcdc, 5) && (wx <= 159) && (m_iWindowLine < 144))
    {
        u8 wy = m_pMemory->Retrieve(0xFF4A);

        if (wy > 143)
            return;

        if (wy <= line)
        {
            int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
            int map = IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800;
            int lineAdjusted = m_iWindowLine;
            int y_32 = (lineAdjusted / 8) * 32;
            int pixely = lineAdjusted % 8;
            int pixely_2 = pixely * 2;

            for (int x = 0; x < 32; x++)
            {
                int tile = 0;

                if (tiles == 0x8800)
                {
                    if (m_bCGB)
                        tile = static_cast<s8> (m_pMemory->ReadCGBLCDRAM(map + y_32 + x, false));
                    else
                        tile = static_cast<s8> (m_pMemory->Retrieve(map + y_32 + x));
                    tile += 128;
                }
                else
                {
                    if (m_bCGB)
                        tile = m_pMemory->ReadCGBLCDRAM(map + y_32 + x, false);
                    else
                        tile = m_pMemory->Retrieve(map + y_32 + x);
                }

                u8 cgb_tile_attr = 0;
                u8 cgb_tile_pal = 0;
                bool cgb_tile_bank = false;
                bool cgb_tile_yflip = false;
                bool cgb_tile_xflip = false;
                bool cgb_tile_priority = false;
                if (m_bCGB)
                {
                    cgb_tile_attr = m_pMemory->ReadCGBLCDRAM(map + y_32 + x, true);
                    cgb_tile_pal = cgb_tile_attr & 0x07;
                    cgb_tile_bank = IsSetBit(cgb_tile_attr, 3);
                    cgb_tile_xflip = IsSetBit(cgb_tile_attr, 5);
                    cgb_tile_yflip = IsSetBit(cgb_tile_attr, 6);
                    cgb_tile_priority = IsSetBit(cgb_tile_attr, 7);
                }

                int mapOffsetX = x * 8;
                int tile_16 = tile * 16;
                u8 byte1 = 0;
                u8 byte2 = 0;

                if (m_bCGB && cgb_tile_yflip)
                {
                    pixely = 7 - pixely;
                    pixely_2 = pixely * 2;
                }

                if (m_bCGB && cgb_tile_bank)
                {
                    byte1 = m_pMemory->ReadCGBLCDRAM(tiles + tile_16 + pixely_2, true);
                    byte2 = m_pMemory->ReadCGBLCDRAM(tiles + tile_16 + pixely_2 + 1, true);
                }
                else
                {
                    byte1 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2);
                    byte2 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2 + 1);
                }

                for (int pixelx = 0; pixelx < 8; pixelx++)
                {
                    int bufferX = (mapOffsetX + pixelx + wx);

                    if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                    {
                        int pixelx_pos = pixelx;

                        if (m_bCGB && cgb_tile_xflip)
                        {
                            pixelx_pos = 7 - pixelx_pos;
                        }

                        int pixel = (byte1 & (0x1 << (7 - pixelx_pos))) ? 1 : 0;
                        pixel |= (byte2 & (0x1 << (7 - pixelx_pos))) ? 2 : 0;

                        int position = (line * GAMEBOY_WIDTH) + bufferX;
                        m_pColorCacheBuffer[position] = pixel & 0x03;
                        if (m_bCGB)
                        {
                            GB_Color color = m_CGBBackgroundPalettes[cgb_tile_pal][pixel];
                            m_pColorFrameBuffer[position] = ConvertTo8BitColor(color);
                        }
                        else
                        {
                            u8 palette = m_pMemory->Retrieve(0xFF47);
                            u8 color = (palette >> (pixel * 2)) & 0x03;
                            m_pFrameBuffer[position] = color;
                        }
                    }
                }
            }
            m_iWindowLine++;
        }
    }
}

void Video::RenderSprites(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (IsSetBit(lcdc, 1))
    {
        int sprite_height = IsSetBit(lcdc, 2) ? 16 : 8;

        for (int sprite = 39; sprite >= 0; sprite--)
        {
            int sprite_4 = sprite * 4;
            int sprite_y = m_pMemory->Retrieve(0xFE00 + sprite_4) - 16;

            if ((sprite_y <= line) && ((sprite_y + sprite_height) > line))
            {
                int sprite_x = m_pMemory->Retrieve(0xFE00 + sprite_4 + 1) - 8;

                if (sprite_x >= -7 && sprite_x < GAMEBOY_WIDTH)
                {
                    int sprite_tile_16 = (m_pMemory->Retrieve(0xFE00 + sprite_4 + 2)
                            & ((sprite_height == 16) ? 0xFE : 0xFF)) * 16;
                    u8 sprite_flags = m_pMemory->Retrieve(0xFE00 + sprite_4 + 3);
                    int sprite_pallette = IsSetBit(sprite_flags, 4) ? 1 : 0;
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
                        pixel_y_2 = (pixel_y - 8) * 2;
                        offset = 16;
                    }
                    else
                        pixel_y_2 = pixel_y * 2;

                    if (m_bCGB && cgb_tile_bank)
                    {
                        byte1 = m_pMemory->ReadCGBLCDRAM(tiles + sprite_tile_16 +
                                pixel_y_2 + offset, true);
                        byte2 = m_pMemory->ReadCGBLCDRAM(tiles + sprite_tile_16 +
                                pixel_y_2 + 1 + offset, true);
                    }
                    else
                    {
                        byte1 = m_pMemory->Retrieve(tiles + sprite_tile_16 +
                                pixel_y_2 + offset);
                        byte2 = m_pMemory->Retrieve(tiles + sprite_tile_16 +
                                pixel_y_2 + 1 + offset);
                    }

                    for (int pixelx = 0; pixelx < 8; pixelx++)
                    {
                        int pixel = (byte1 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 1 : 0;
                        pixel |= (byte2 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 2 : 0;

                        if (pixel)
                        {
                            int bufferX = (sprite_x + pixelx);
                            u8 color_cache = m_pColorCacheBuffer[(line * GAMEBOY_WIDTH) + bufferX];
                            int sprite_x_cache = m_pSpriteXCacheBuffer[(line * GAMEBOY_WIDTH) + bufferX];

                            if ((color_cache & 0x10) != 0)
                            {
                                if (!m_bCGB && (sprite_x_cache < sprite_x))
                                    continue;
                            }

                            if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                            {
                                if (aboveBG || ((color_cache & 0x03) == 0))
                                {
                                    int position = (line * GAMEBOY_WIDTH) + bufferX;
                                    m_pColorCacheBuffer[position] = (color_cache & 0x03) | 0x10;
                                    m_pSpriteXCacheBuffer[position] = sprite_x;
                                    if (m_bCGB)
                                    {
                                        GB_Color color = m_CGBSpritePalettes[cgb_tile_pal][pixel];
                                        m_pColorFrameBuffer[position] = ConvertTo8BitColor(color);
                                    }
                                    else
                                    {
                                        u8 palette = m_pMemory->Retrieve(sprite_pallette ? 0xFF49 : 0xFF48);
                                        u8 color = (palette >> (pixel * 2)) & 0x03;
                                        m_pFrameBuffer[position] = color;
                                    }
                                }
                            }
                        }
                    }
                }
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
                u8 signal = m_pProcessor->GetIRQ48Signal();
                if (signal == 0x00)
                {
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                }
                m_pProcessor->SetIRQ48Signal(SetBit(signal, 3));
            }
        }
        else
        {
            stat = UnsetBit(stat, 2);
            m_pProcessor->SetIRQ48Signal(UnsetBit(m_pProcessor->GetIRQ48Signal(), 3));
        }

        m_pMemory->Load(0xFF41, stat);
    }
}

GB_Color Video::ConvertTo8BitColor(GB_Color color)
{
    color.red = (color.red * 255) / 31;
    color.green = (color.green * 255) / 31;
    color.blue = (color.blue * 255) / 31;
    color.alpha = 0xFF;

    return color;
}
