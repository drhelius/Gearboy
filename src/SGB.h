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

#ifndef SGB_H
#define SGB_H

#include "definitions.h"

#define SGB_SCREEN_WIDTH 256
#define SGB_SCREEN_HEIGHT 224
#define SGB_PACKET_SIZE 16
#define SGB_MAX_PACKETS 7
#define SGB_MAX_COMMAND_SIZE (SGB_PACKET_SIZE * SGB_MAX_PACKETS)
#define SGB_ATTR_MAP_WIDTH 20
#define SGB_ATTR_MAP_HEIGHT 18
#define SGB_ATF_COUNT 45
#define SGB_ATF_SIZE 90
#define SGB_SYSTEM_PALETTE_COUNT 512
#define SGB_BORDER_TILE_DATA_SIZE (256 * 32)

class Memory;
class Video;

class SGB
{
public:
    enum MaskMode
    {
        MaskDisabled = 0,
        MaskFreeze = 1,
        MaskBlack = 2,
        MaskColor0 = 3
    };

    enum TransferDest
    {
        TransferLowTiles,
        TransferHighTiles,
        TransferBorderData,
        TransferPalettes,
        TransferAttributes
    };

    enum Command
    {
        PAL01 = 0x00,
        PAL23 = 0x01,
        PAL03 = 0x02,
        PAL12 = 0x03,
        ATTR_BLK = 0x04,
        ATTR_LIN = 0x05,
        ATTR_DIV = 0x06,
        ATTR_CHR = 0x07,
        SOUND = 0x08,
        SOU_TRN = 0x09,
        PAL_SET = 0x0A,
        PAL_TRN = 0x0B,
        DATA_SND = 0x0F,
        MLT_REQ = 0x11,
        CHR_TRN = 0x13,
        PCT_TRN = 0x14,
        ATTR_TRN = 0x15,
        ATTR_SET = 0x16,
        MASK_EN = 0x17,
    };

    struct Border
    {
        u8 tiles[SGB_BORDER_TILE_DATA_SIZE];
        u16 map[32 * 32];
        u16 palette[16 * 4];
    };

public:
    SGB(Memory* pMemory, Video* pVideo);
    ~SGB();
    void Init();
    void Reset();
    void WriteJOYP(u8 value);
    void Render(u16* pFrameBuffer, GB_Color_Format pixelFormat, bool incomplete);
    void CopyScreenBuffer(const u8* pVideoFrameBuffer);
    int GetPlayerCount() const;
    int GetCurrentPlayer() const;
    MaskMode GetMaskMode() const;
    u8* GetScreenBuffer();
    u16 GetCommandWriteIndex() const;
    bool IsReadyForPulse() const;
    bool IsReadyForWrite() const;
    bool IsReadyForStop() const;
    bool AreCommandsDisabled() const;
    u8 GetVRAMTransferCountdown() const;
    u8 GetTransferDest() const;
    u8 GetBorderAnimation() const;
    bool IsBorderEmpty() const;
    const u16* GetEffectivePalettes() const;
    const u16* GetSystemPalettes() const;
    const u8* GetAttributeMap() const;
    const u8* GetAttributeFiles() const;
    const Border* GetBorder() const;
    const Border* GetPendingBorder() const;
    const u8* GetCommand() const;
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    void CommandReady();
    void CommandPAL(int first, int second);
    void CommandATTR_BLK();
    void CommandATTR_LIN();
    void CommandATTR_DIV();
    void CommandATTR_CHR();
    void CommandPAL_SET();
    void CommandPAL_TRN();
    void CommandMLT_REQ();
    void CommandCHR_TRN();
    void CommandPCT_TRN();
    void CommandATTR_TRN();
    void CommandATTR_SET();
    void CommandMASK_EN();
    void PerformVRAMTransfer();
    void LoadAttributeFile(int fileIndex);
    void RenderGameScreen(u16* pFrameBuffer, GB_Color_Format pixelFormat);
    void RenderBorder(u16* pFrameBuffer, GB_Color_Format pixelFormat);
    u16 ConvertRGB15(u16 color, GB_Color_Format pixelFormat);
    void LoadDefaultBorder();

private:
    Memory* m_pMemory;
    Video* m_pVideo;

    u8 m_Command[SGB_MAX_COMMAND_SIZE];
    u16 m_CommandWriteIndex;
    bool m_bReadyForPulse;
    bool m_bReadyForWrite;
    bool m_bReadyForStop;
    bool m_bDisableCommands;

    u8 m_ScreenBuffer[160 * 144];
    u8 m_EffectiveScreenBuffer[160 * 144];

    u8 m_iPlayerCount;
    u8 m_iCurrentPlayer;

    u8 m_MaskMode;

    u8 m_iVRAMTransferCountdown;
    u8 m_TransferDest;

    u16 m_EffectivePalettes[4 * 4];
    u16 m_SystemPalettes[SGB_SYSTEM_PALETTE_COUNT * 4];

    u8 m_AttributeMap[SGB_ATTR_MAP_WIDTH * SGB_ATTR_MAP_HEIGHT];
    u8 m_AttributeFiles[SGB_ATF_COUNT * SGB_ATF_SIZE];
    u8 m_AttributeFilesPadding[0xFE0 - (SGB_ATF_COUNT * SGB_ATF_SIZE)];

    Border m_BorderBuffers[2];
    Border* m_pBorder;
    Border* m_pPendingBorder;
    u8 m_iBorderAnimation;
    bool m_bBorderEmpty;

    u8 m_PreviousJOYP;
};

#endif /* SGB_H */
