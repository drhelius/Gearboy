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

#include "SGB.h"
#include "Memory.h"
#include "Video.h"

SGB::SGB(Memory* pMemory, Video* pVideo)
{
    m_pMemory = pMemory;
    m_pVideo = pVideo;
}

SGB::~SGB()
{
}

void SGB::Init()
{
    Reset();
}

void SGB::Reset()
{
    memset(m_Command, 0, sizeof(m_Command));
    m_CommandWriteIndex = 0;
    m_bReadyForPulse = true;
    m_bReadyForWrite = false;
    m_bReadyForStop = false;
    m_bDisableCommands = false;

    memset(m_ScreenBuffer, 0, sizeof(m_ScreenBuffer));
    memset(m_EffectiveScreenBuffer, 0, sizeof(m_EffectiveScreenBuffer));

    m_iPlayerCount = 1;
    m_iCurrentPlayer = 0;

    m_MaskMode = MaskBlack;

    m_iVRAMTransferCountdown = 0;
    m_TransferDest = TransferLowTiles;

    memset(m_EffectivePalettes, 0, sizeof(m_EffectivePalettes));
    memset(m_SystemPalettes, 0, sizeof(m_SystemPalettes));
    memset(m_AttributeMap, 0, sizeof(m_AttributeMap));
    memset(m_AttributeFiles, 0, sizeof(m_AttributeFiles));
    memset(m_AttributeFilesPadding, 0, sizeof(m_AttributeFilesPadding));

    memset(&m_BorderBuffers[0], 0, sizeof(Border));
    memset(&m_BorderBuffers[1], 0, sizeof(Border));
    m_pBorder = &m_BorderBuffers[0];
    m_pPendingBorder = &m_BorderBuffers[1];
    m_iBorderAnimation = 0;
    m_bBorderEmpty = true;

    m_PreviousJOYP = 0xCF;

    LoadDefaultBorder();
}

void SGB::WriteJOYP(u8 value)
{
    if (m_bDisableCommands)
        return;

    u16 command_size = (m_Command[0] & 7) * SGB_PACKET_SIZE * 8;
    if (command_size == 0)
        command_size = SGB_PACKET_SIZE * 8;

    if ((m_Command[0] & 0xF1) == 0xF1)
        command_size = SGB_PACKET_SIZE * 8;

    u8 old_joyp = m_PreviousJOYP;
    m_PreviousJOYP = value;

    if ((value & 0x20) != 0 && (old_joyp & 0x20) == 0)
    {
        if ((m_iPlayerCount & 1) == 0)
        {
            m_iCurrentPlayer++;
            m_iCurrentPlayer &= (m_iPlayerCount - 1);
        }
    }

    switch ((value >> 4) & 3)
    {
        case 3:
            m_bReadyForPulse = true;
            break;

        case 2:
            if (!m_bReadyForPulse || !m_bReadyForWrite)
                return;
            if (m_bReadyForStop)
            {
                if (m_CommandWriteIndex == command_size)
                {
                    CommandReady();
                    m_CommandWriteIndex = 0;
                    memset(m_Command, 0, sizeof(m_Command));
                }
                m_bReadyForPulse = false;
                m_bReadyForWrite = false;
                m_bReadyForStop = false;
            }
            else
            {
                if (m_CommandWriteIndex < sizeof(m_Command) * 8)
                {
                    m_CommandWriteIndex++;
                    m_bReadyForPulse = false;
                    if (((m_CommandWriteIndex) & (SGB_PACKET_SIZE * 8 - 1)) == 0)
                        m_bReadyForStop = true;
                }
            }
            break;

        case 1:
            if (!m_bReadyForPulse || !m_bReadyForWrite)
                return;

            if (m_bReadyForStop)
            {
                Debug("Corrupt SGB command.");
                m_bReadyForPulse = false;
                m_bReadyForWrite = false;
                m_CommandWriteIndex = 0;
                memset(m_Command, 0, sizeof(m_Command));
            }
            else
            {
                if (m_CommandWriteIndex < sizeof(m_Command) * 8)
                {
                    m_Command[m_CommandWriteIndex / 8] |= 1 << (m_CommandWriteIndex & 7);
                    m_CommandWriteIndex++;
                    m_bReadyForPulse = false;
                    if (((m_CommandWriteIndex) & (SGB_PACKET_SIZE * 8 - 1)) == 0)
                        m_bReadyForStop = true;
                }
            }
            break;

        case 0:
            if (!m_bReadyForPulse)
                return;
            m_bReadyForWrite = true;
            m_bReadyForPulse = false;
            if (((m_CommandWriteIndex) & (SGB_PACKET_SIZE * 8 - 1)) != 0 ||
                m_CommandWriteIndex == 0 || m_bReadyForStop)
            {
                m_CommandWriteIndex = 0;
                memset(m_Command, 0, sizeof(m_Command));
                m_bReadyForStop = false;
            }
            break;
    }
}

void SGB::Render(u16* pFrameBuffer, GB_Color_Format pixelFormat, bool incomplete)
{
    if (m_bBorderEmpty)
        memset(pFrameBuffer, 0, SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT * sizeof(u16));

    if (m_iVRAMTransferCountdown > 0)
    {
        if (--m_iVRAMTransferCountdown == 0)
        {
            Debug("SGB VRAM transfer complete: dest=%d", m_TransferDest);
            PerformVRAMTransfer();
        }
    }

    if (m_iBorderAnimation > 0)
    {
        if (m_bBorderEmpty)
        {
            Border* temp = m_pBorder;
            m_pBorder = m_pPendingBorder;
            m_pPendingBorder = temp;
            m_bBorderEmpty = false;
            m_iBorderAnimation = 32;
        }
        else
        {
            m_iBorderAnimation--;
            if (m_iBorderAnimation == 32)
            {
                Border* temp = m_pBorder;
                m_pBorder = m_pPendingBorder;
                m_pPendingBorder = temp;
            }
        }
    }

    if (m_MaskMode != MaskFreeze && !incomplete)
        memcpy(m_EffectiveScreenBuffer, m_ScreenBuffer, sizeof(m_EffectiveScreenBuffer));

    switch ((MaskMode)m_MaskMode)
    {
        case MaskDisabled:
        case MaskFreeze:
            RenderGameScreen(pFrameBuffer, pixelFormat);
            break;
        case MaskBlack:
        {
            int offsetX = (SGB_SCREEN_WIDTH - GAMEBOY_WIDTH) / 2;
            int offsetY = (SGB_SCREEN_HEIGHT - GAMEBOY_HEIGHT) / 2;
            for (int y = 0; y < GAMEBOY_HEIGHT; y++)
            {
                u16* row = &pFrameBuffer[(offsetX) + (offsetY + y) * SGB_SCREEN_WIDTH];
                memset(row, 0, GAMEBOY_WIDTH * sizeof(u16));
            }
            break;
        }
        case MaskColor0:
        {
            u16 color0 = ConvertRGB15(m_EffectivePalettes[0], pixelFormat);
            int offsetX = (SGB_SCREEN_WIDTH - GAMEBOY_WIDTH) / 2;
            int offsetY = (SGB_SCREEN_HEIGHT - GAMEBOY_HEIGHT) / 2;
            for (int y = 0; y < GAMEBOY_HEIGHT; y++)
            {
                u16* row = &pFrameBuffer[(offsetX) + (offsetY + y) * SGB_SCREEN_WIDTH];
                for (int x = 0; x < GAMEBOY_WIDTH; x++)
                    row[x] = color0;
            }
            break;
        }
    }

    if (!m_bBorderEmpty)
        RenderBorder(pFrameBuffer, pixelFormat);
}

void SGB::CopyScreenBuffer(const u8* pVideoFrameBuffer)
{
    memcpy(m_ScreenBuffer, pVideoFrameBuffer, sizeof(m_ScreenBuffer));
}

int SGB::GetPlayerCount() const
{
    return m_iPlayerCount;
}

int SGB::GetCurrentPlayer() const
{
    return m_iCurrentPlayer;
}

SGB::MaskMode SGB::GetMaskMode() const
{
    return (MaskMode)m_MaskMode;
}

u8* SGB::GetScreenBuffer()
{
    return m_ScreenBuffer;
}

u16 SGB::GetCommandWriteIndex() const
{
    return m_CommandWriteIndex;
}

bool SGB::IsReadyForPulse() const
{
    return m_bReadyForPulse;
}

bool SGB::IsReadyForWrite() const
{
    return m_bReadyForWrite;
}

bool SGB::IsReadyForStop() const
{
    return m_bReadyForStop;
}

bool SGB::AreCommandsDisabled() const
{
    return m_bDisableCommands;
}

u8 SGB::GetVRAMTransferCountdown() const
{
    return m_iVRAMTransferCountdown;
}

u8 SGB::GetTransferDest() const
{
    return m_TransferDest;
}

u8 SGB::GetBorderAnimation() const
{
    return m_iBorderAnimation;
}

bool SGB::IsBorderEmpty() const
{
    return m_bBorderEmpty;
}

const u16* SGB::GetEffectivePalettes() const
{
    return m_EffectivePalettes;
}

const u16* SGB::GetSystemPalettes() const
{
    return m_SystemPalettes;
}

const u8* SGB::GetAttributeMap() const
{
    return m_AttributeMap;
}

const u8* SGB::GetAttributeFiles() const
{
    return m_AttributeFiles;
}

const SGB::Border* SGB::GetBorder() const
{
    return m_pBorder;
}

const SGB::Border* SGB::GetPendingBorder() const
{
    return m_pPendingBorder;
}

const u8* SGB::GetCommand() const
{
    return m_Command;
}

void SGB::CommandReady()
{
    if ((m_Command[0] & 0xF1) == 0xF1)
        return;

    if ((m_Command[0] & 7) == 0)
        return;

    u8 commandCode = m_Command[0] >> 3;

    Debug("SGB command: 0x%02X (length=%d)", commandCode, m_Command[0] & 7);

    switch (commandCode)
    {
        case PAL01:
            CommandPAL(0, 1);
            break;
        case PAL23:
            CommandPAL(2, 3);
            break;
        case PAL03:
            CommandPAL(0, 3);
            break;
        case PAL12:
            CommandPAL(1, 2);
            break;
        case ATTR_BLK:
            CommandATTR_BLK();
            break;
        case ATTR_LIN:
            CommandATTR_LIN();
            break;
        case ATTR_DIV:
            CommandATTR_DIV();
            break;
        case ATTR_CHR:
            CommandATTR_CHR();
            break;
        case PAL_SET:
            CommandPAL_SET();
            break;
        case PAL_TRN:
            CommandPAL_TRN();
            break;
        case DATA_SND:
            break;
        case MLT_REQ:
            CommandMLT_REQ();
            break;
        case CHR_TRN:
            CommandCHR_TRN();
            break;
        case PCT_TRN:
            CommandPCT_TRN();
            break;
        case ATTR_TRN:
            CommandATTR_TRN();
            break;
        case ATTR_SET:
            CommandATTR_SET();
            break;
        case MASK_EN:
            CommandMASK_EN();
            break;
        default:
            if (commandCode == SOUND && (m_Command[1] & ~0x80) == 0 && (m_Command[2] & ~0x80) == 0)
                break;
            Debug("Unimplemented SGB command: 0x%02X", commandCode);
            break;
    }
}

void SGB::CommandPAL(int first, int second)
{
    u16 color0;
    memcpy(&color0, &m_Command[1], 2);
    m_EffectivePalettes[0] = color0;
    m_EffectivePalettes[4] = color0;
    m_EffectivePalettes[8] = color0;
    m_EffectivePalettes[12] = color0;

    for (int i = 0; i < 3; i++)
        memcpy(&m_EffectivePalettes[first * 4 + i + 1], &m_Command[3 + i * 2], 2);

    for (int i = 0; i < 3; i++)
        memcpy(&m_EffectivePalettes[second * 4 + i + 1], &m_Command[9 + i * 2], 2);
}

void SGB::CommandATTR_BLK()
{
    int count = m_Command[1];
    if (count > 0x12)
        return;

    for (int i = 0; i < count; i++)
    {
        int offset = 2 + i * 6;
        u8 control = m_Command[offset];
        u8 palettes = m_Command[offset + 1];

        bool inside = control & 1;
        bool middle = control & 2;
        bool outside = control & 4;

        u8 inside_pal = palettes & 0x3;
        u8 middle_pal = (palettes >> 2) & 0x3;
        u8 outside_pal = (palettes >> 4) & 0x3;

        if (inside && !middle && !outside)
        {
            middle = true;
            middle_pal = inside_pal;
        }
        else if (outside && !middle && !inside)
        {
            middle = true;
            middle_pal = outside_pal;
        }

        u8 left = m_Command[offset + 2] & 0x1F;
        u8 top = m_Command[offset + 3] & 0x1F;
        u8 right = m_Command[offset + 4] & 0x1F;
        u8 bottom = m_Command[offset + 5] & 0x1F;

        for (int y = 0; y < SGB_ATTR_MAP_HEIGHT; y++)
        {
            for (int x = 0; x < SGB_ATTR_MAP_WIDTH; x++)
            {
                if (x < left || x > right || y < top || y > bottom)
                {
                    if (outside)
                        m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = outside_pal;
                }
                else if (x > left && x < right && y > top && y < bottom)
                {
                    if (inside)
                        m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = inside_pal;
                }
                else if (middle)
                {
                    m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = middle_pal;
                }
            }
        }
    }
}

void SGB::CommandATTR_LIN()
{
    int count = m_Command[1];
    if (count > (int)sizeof(m_Command) - 2)
        return;

    for (int i = 0; i < count; i++)
    {
        u8 data = m_Command[2 + i];
        bool horizontal = data & 0x80;
        u8 palette = (data >> 5) & 0x3;
        u8 line = data & 0x1F;

        if (horizontal)
        {
            if (line >= SGB_ATTR_MAP_HEIGHT) continue;
            for (int x = 0; x < SGB_ATTR_MAP_WIDTH; x++)
                m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * line] = palette;
        }
        else
        {
            if (line >= SGB_ATTR_MAP_WIDTH) continue;
            for (int y = 0; y < SGB_ATTR_MAP_HEIGHT; y++)
                m_AttributeMap[line + SGB_ATTR_MAP_WIDTH * y] = palette;
        }
    }
}

void SGB::CommandATTR_DIV()
{
    u8 data = m_Command[1];
    u8 high_pal = data & 0x3;
    u8 low_pal = (data >> 2) & 0x3;
    u8 middle_pal = (data >> 4) & 0x3;
    bool horizontal = data & 0x40;
    u8 line = m_Command[2] & 0x1F;

    for (int y = 0; y < SGB_ATTR_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < SGB_ATTR_MAP_WIDTH; x++)
        {
            u8 coord = horizontal ? y : x;
            if (coord < line)
                m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = low_pal;
            else if (coord == line)
                m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = middle_pal;
            else
                m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = high_pal;
        }
    }
}

void SGB::CommandATTR_CHR()
{
    u8 startX = m_Command[1];
    u8 startY = m_Command[2];
    u16 count;
    memcpy(&count, &m_Command[3], 2);
#ifdef IS_BIG_ENDIAN
    count = (count >> 8) | (count << 8);
#endif
    u8 direction = m_Command[5];

    if (startX >= SGB_ATTR_MAP_WIDTH || startY >= SGB_ATTR_MAP_HEIGHT)
        return;

    u8 x = startX;
    u8 y = startY;

    for (u16 i = 0; i < count; i++)
    {
        u8 palette = (m_Command[6 + i / 4] >> (((~i) & 3) << 1)) & 3;
        m_AttributeMap[x + SGB_ATTR_MAP_WIDTH * y] = palette;

        if (direction)
        {
            y++;
            if (y >= SGB_ATTR_MAP_HEIGHT)
            {
                x++;
                y = 0;
                if (x >= SGB_ATTR_MAP_WIDTH) break;
            }
        }
        else
        {
            x++;
            if (x >= SGB_ATTR_MAP_WIDTH)
            {
                y++;
                x = 0;
                if (y >= SGB_ATTR_MAP_HEIGHT) break;
            }
        }
    }
}

void SGB::CommandPAL_SET()
{
    Debug("SGB PAL_SET ids: %03X %03X %03X %03X flags=%02X",
        m_Command[1] | ((m_Command[2] & 0x01) << 8),
        m_Command[3] | ((m_Command[4] & 0x01) << 8),
        m_Command[5] | ((m_Command[6] & 0x01) << 8),
        m_Command[7] | ((m_Command[8] & 0x01) << 8),
        m_Command[9]);

    for (int p = 0; p < 4; p++)
    {
        u16 paletteId = m_Command[1 + p * 2] | ((m_Command[2 + p * 2] & 0x01) << 8);
        if (paletteId < SGB_SYSTEM_PALETTE_COUNT)
            memcpy(&m_EffectivePalettes[p * 4], &m_SystemPalettes[paletteId * 4], 8);
    }

    m_EffectivePalettes[12] = m_EffectivePalettes[8] =
        m_EffectivePalettes[4] = m_EffectivePalettes[0];

    if (m_Command[9] & 0x80)
        LoadAttributeFile(m_Command[9] & 0x3F);

    if (m_Command[9] & 0x40)
    {
        m_MaskMode = MaskDisabled;
        Debug("SGB PAL_SET: cancel mask");
    }
}

void SGB::CommandPAL_TRN()
{
    m_iVRAMTransferCountdown = 3;
    m_TransferDest = TransferPalettes;
}

void SGB::CommandMLT_REQ()
{
    m_iPlayerCount = (m_Command[1] & 3) + 1;
    if (m_iPlayerCount == 3)
        m_iPlayerCount = 4;

    m_iCurrentPlayer &= (m_iPlayerCount - 1);
}

void SGB::CommandCHR_TRN()
{
    m_iVRAMTransferCountdown = 3;
    m_TransferDest = (m_Command[1] & 1) ? TransferHighTiles : TransferLowTiles;
}

void SGB::CommandPCT_TRN()
{
    m_iVRAMTransferCountdown = 3;
    m_TransferDest = TransferBorderData;
}

void SGB::CommandATTR_TRN()
{
    m_iVRAMTransferCountdown = 3;
    m_TransferDest = TransferAttributes;
}

void SGB::CommandATTR_SET()
{
    LoadAttributeFile(m_Command[1] & 0x3F);

    if (m_Command[1] & 0x40)
    {
        m_MaskMode = MaskDisabled;
        Debug("SGB ATTR_SET: cancel mask");
    }
}

void SGB::CommandMASK_EN()
{
    m_MaskMode = m_Command[1] & 3;
    Debug("SGB MASK_EN: mode=%d", m_MaskMode);
}

void SGB::PerformVRAMTransfer()
{
    u16* data = NULL;
    int size = 0;

    switch ((TransferDest)m_TransferDest)
    {
        case TransferLowTiles:
            size = 0x100;
            data = (u16*)m_pPendingBorder->tiles;
            break;
        case TransferHighTiles:
            size = 0x100;
            data = (u16*)(m_pPendingBorder->tiles + 0x1000);
            break;
        case TransferPalettes:
            size = 0x100;
            data = m_SystemPalettes;
            break;
        case TransferBorderData:
            size = 0x88;
            data = (u16*)m_pPendingBorder->map;
            break;
        case TransferAttributes:
            size = 0xFE;
            data = (u16*)m_AttributeFiles;
            break;
        default:
            return;
    }

    const u8* screenBuffer = m_ScreenBuffer;

    for (int tile = 0; tile < size; tile++)
    {
        int tile_x = (tile % 20) * 8;
        int tile_y = (tile / 20) * 8;

        for (int y = 0; y < 8; y++)
        {
            static const u16 kPixelToBits[4] = {0x0000, 0x0080, 0x8000, 0x8080};
            u16 word = 0;

            for (int x = 0; x < 8; x++)
            {
                word |= kPixelToBits[screenBuffer[(tile_x + x) + (tile_y + y) * 160] & 3] >> x;
            }

            if (m_TransferDest == TransferPalettes || m_TransferDest == TransferBorderData)
            {
#ifdef IS_BIG_ENDIAN
                word = (word >> 8) | (word << 8);
#endif
            }

            *data = word;
            data++;
        }
    }

    if (m_TransferDest == TransferBorderData)
        m_iBorderAnimation = 105;
}

void SGB::LoadAttributeFile(int fileIndex)
{
    if (fileIndex > 0x2C)
        return;

    u8* output = m_AttributeMap;
    for (int i = 0; i < SGB_ATF_SIZE; i++)
    {
        u8 byte = m_AttributeFiles[fileIndex * SGB_ATF_SIZE + i];
        for (int j = 3; j >= 0; j--)
        {
            *(output++) = byte >> 6;
            byte <<= 2;
        }
    }
}

void SGB::RenderGameScreen(u16* pFrameBuffer, GB_Color_Format pixelFormat)
{
    u16 colors[4 * 4];
    for (int i = 0; i < 4 * 4; i++)
    {
        u16 rawColor = m_EffectivePalettes[i];
#ifdef IS_BIG_ENDIAN
        rawColor = (rawColor >> 8) | (rawColor << 8);
#endif
        colors[i] = ConvertRGB15(rawColor, pixelFormat);
    }

    int offsetX = (SGB_SCREEN_WIDTH - GAMEBOY_WIDTH) / 2;
    int offsetY = (SGB_SCREEN_HEIGHT - GAMEBOY_HEIGHT) / 2;

    u8* input = m_EffectiveScreenBuffer;

    for (int y = 0; y < GAMEBOY_HEIGHT; y++)
    {
        int attrRow = (y >> 3) * SGB_ATTR_MAP_WIDTH;
        u16* row = &pFrameBuffer[(offsetX) + (offsetY + y) * SGB_SCREEN_WIDTH];

        for (int x = 0; x < GAMEBOY_WIDTH; x++)
        {
            u8 palette = m_AttributeMap[attrRow + (x >> 3)] & 3;
            u8 colorIndex = (*(input++)) & 3;
            row[x] = colors[colorIndex + palette * 4];
        }
    }
}

void SGB::RenderBorder(u16* pFrameBuffer, GB_Color_Format pixelFormat)
{
    u16 gameRawColor0 = m_EffectivePalettes[0];
#ifdef IS_BIG_ENDIAN
    gameRawColor0 = (gameRawColor0 >> 8) | (gameRawColor0 << 8);
#endif
    u16 gameColor0 = ConvertRGB15(gameRawColor0, pixelFormat);

    u8 fade = 0;
    if (m_iBorderAnimation > 0 && m_iBorderAnimation <= 64)
    {
        if (m_iBorderAnimation > 32)
            fade = 64 - m_iBorderAnimation;
        else
            fade = m_iBorderAnimation;
    }

    u16 borderColors[16 * 4];
    for (int i = 0; i < 16 * 4; i++)
    {
        u16 rawColor = m_pBorder->palette[i];
#ifdef IS_BIG_ENDIAN
        rawColor = (rawColor >> 8) | (rawColor << 8);
#endif
        if (fade > 0)
        {
            u8 r = (rawColor) & 0x1F;
            u8 g = (rawColor >> 5) & 0x1F;
            u8 b = (rawColor >> 10) & 0x1F;
            r = (r > fade) ? r - fade : 0;
            g = (g > fade) ? g - fade : 0;
            b = (b > fade) ? b - fade : 0;
            rawColor = r | (g << 5) | (b << 10);
        }
        borderColors[i] = ConvertRGB15(rawColor, pixelFormat);
    }

    for (int tile_y = 0; tile_y < 28; tile_y++)
    {
        for (int tile_x = 0; tile_x < 32; tile_x++)
        {
            bool gbArea = (tile_x >= 6 && tile_x < 26 && tile_y >= 5 && tile_y < 23);

            u16 tileEntry = m_pBorder->map[tile_x + tile_y * 32];
#ifdef IS_BIG_ENDIAN
            tileEntry = (tileEntry >> 8) | (tileEntry << 8);
#endif

            if (tileEntry & 0x300)
                continue;

            u8 flipX = (tileEntry & 0x4000) ? 0 : 7;
            u8 flipY = (tileEntry & 0x8000) ? 7 : 0;
            u8 palette = (tileEntry >> 10) & 3;

            for (int y = 0; y < 8; y++)
            {
                int base = (tileEntry & 0xFF) * 32 + (y ^ flipY) * 2;

                for (int x = 0; x < 8; x++)
                {
                    u8 bit = 1 << (x ^ flipX);

                    u8 color = ((m_pBorder->tiles[base]      & bit) ? 1 : 0) |
                               ((m_pBorder->tiles[base + 1]  & bit) ? 2 : 0) |
                               ((m_pBorder->tiles[base + 16] & bit) ? 4 : 0) |
                               ((m_pBorder->tiles[base + 17] & bit) ? 8 : 0);

                    int outX = tile_x * 8 + x;
                    int outY = tile_y * 8 + y;

                    u16* output = &pFrameBuffer[outX + outY * SGB_SCREEN_WIDTH];

                    if (color == 0)
                    {
                        if (gbArea)
                            continue;
                        *output = gameColor0;
                    }
                    else
                        *output = borderColors[color + palette * 16];
                }
            }
        }
    }
}

u16 SGB::ConvertRGB15(u16 color, GB_Color_Format pixelFormat)
{
    u8 r5 = (color) & 0x1F;
    u8 g5 = (color >> 5) & 0x1F;
    u8 b5 = (color >> 10) & 0x1F;

    switch (pixelFormat)
    {
        case GB_PIXEL_RGB565:
        {
            u8 g6 = (g5 << 1) | (g5 >> 4);
            return (r5 << 11) | (g6 << 5) | b5;
        }
        case GB_PIXEL_RGB555:
            return 0x8000 | (r5 << 10) | (g5 << 5) | b5;
        case GB_PIXEL_BGR565:
        {
            u8 g6 = (g5 << 1) | (g5 >> 4);
            return (b5 << 11) | (g6 << 5) | r5;
        }
        case GB_PIXEL_BGR555:
            return 0x8000 | (b5 << 10) | (g5 << 5) | r5;
        default:
            return 0;
    }
}

void SGB::LoadDefaultBorder()
{
    memset(m_pBorder, 0, sizeof(Border));
    memset(m_pPendingBorder, 0, sizeof(Border));
}

void SGB::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*>(m_Command), sizeof(m_Command));
    stream.write(reinterpret_cast<const char*>(&m_CommandWriteIndex), sizeof(m_CommandWriteIndex));
    stream.write(reinterpret_cast<const char*>(&m_bReadyForPulse), sizeof(m_bReadyForPulse));
    stream.write(reinterpret_cast<const char*>(&m_bReadyForWrite), sizeof(m_bReadyForWrite));
    stream.write(reinterpret_cast<const char*>(&m_bReadyForStop), sizeof(m_bReadyForStop));
    stream.write(reinterpret_cast<const char*>(&m_bDisableCommands), sizeof(m_bDisableCommands));

    stream.write(reinterpret_cast<const char*>(m_ScreenBuffer), sizeof(m_ScreenBuffer));
    stream.write(reinterpret_cast<const char*>(m_EffectiveScreenBuffer), sizeof(m_EffectiveScreenBuffer));

    stream.write(reinterpret_cast<const char*>(&m_iPlayerCount), sizeof(m_iPlayerCount));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentPlayer), sizeof(m_iCurrentPlayer));
    stream.write(reinterpret_cast<const char*>(&m_MaskMode), sizeof(m_MaskMode));

    stream.write(reinterpret_cast<const char*>(&m_iVRAMTransferCountdown), sizeof(m_iVRAMTransferCountdown));
    stream.write(reinterpret_cast<const char*>(&m_TransferDest), sizeof(m_TransferDest));

    stream.write(reinterpret_cast<const char*>(m_EffectivePalettes), sizeof(m_EffectivePalettes));
    stream.write(reinterpret_cast<const char*>(m_SystemPalettes), sizeof(m_SystemPalettes));
    stream.write(reinterpret_cast<const char*>(m_AttributeMap), sizeof(m_AttributeMap));
    stream.write(reinterpret_cast<const char*>(m_AttributeFiles), sizeof(m_AttributeFiles));

    stream.write(reinterpret_cast<const char*>(m_pBorder), sizeof(Border));
    stream.write(reinterpret_cast<const char*>(m_pPendingBorder), sizeof(Border));
    stream.write(reinterpret_cast<const char*>(&m_iBorderAnimation), sizeof(m_iBorderAnimation));
    stream.write(reinterpret_cast<const char*>(&m_bBorderEmpty), sizeof(m_bBorderEmpty));

    stream.write(reinterpret_cast<const char*>(&m_PreviousJOYP), sizeof(m_PreviousJOYP));
}

void SGB::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*>(m_Command), sizeof(m_Command));
    stream.read(reinterpret_cast<char*>(&m_CommandWriteIndex), sizeof(m_CommandWriteIndex));
    stream.read(reinterpret_cast<char*>(&m_bReadyForPulse), sizeof(m_bReadyForPulse));
    stream.read(reinterpret_cast<char*>(&m_bReadyForWrite), sizeof(m_bReadyForWrite));
    stream.read(reinterpret_cast<char*>(&m_bReadyForStop), sizeof(m_bReadyForStop));
    stream.read(reinterpret_cast<char*>(&m_bDisableCommands), sizeof(m_bDisableCommands));

    stream.read(reinterpret_cast<char*>(m_ScreenBuffer), sizeof(m_ScreenBuffer));
    stream.read(reinterpret_cast<char*>(m_EffectiveScreenBuffer), sizeof(m_EffectiveScreenBuffer));

    stream.read(reinterpret_cast<char*>(&m_iPlayerCount), sizeof(m_iPlayerCount));
    stream.read(reinterpret_cast<char*>(&m_iCurrentPlayer), sizeof(m_iCurrentPlayer));
    stream.read(reinterpret_cast<char*>(&m_MaskMode), sizeof(m_MaskMode));

    stream.read(reinterpret_cast<char*>(&m_iVRAMTransferCountdown), sizeof(m_iVRAMTransferCountdown));
    stream.read(reinterpret_cast<char*>(&m_TransferDest), sizeof(m_TransferDest));

    stream.read(reinterpret_cast<char*>(m_EffectivePalettes), sizeof(m_EffectivePalettes));
    stream.read(reinterpret_cast<char*>(m_SystemPalettes), sizeof(m_SystemPalettes));
    stream.read(reinterpret_cast<char*>(m_AttributeMap), sizeof(m_AttributeMap));
    stream.read(reinterpret_cast<char*>(m_AttributeFiles), sizeof(m_AttributeFiles));

    stream.read(reinterpret_cast<char*>(m_pBorder), sizeof(Border));
    stream.read(reinterpret_cast<char*>(m_pPendingBorder), sizeof(Border));
    stream.read(reinterpret_cast<char*>(&m_iBorderAnimation), sizeof(m_iBorderAnimation));
    stream.read(reinterpret_cast<char*>(&m_bBorderEmpty), sizeof(m_bBorderEmpty));

    stream.read(reinterpret_cast<char*>(&m_PreviousJOYP), sizeof(m_PreviousJOYP));
}
