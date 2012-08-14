#include "Video.h"
#include "Memory.h"
#include "Processor.h"

Video::Video(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    InitPointer(m_pFrameBuffer);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 0;
    m_bScreenEnabled = true;
}

void Video::Init()
{
    Reset();
}

void Video::Reset()
{
    m_iStatusMode = 1;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_iStatusModeLYCounter = 144;
    m_bScreenEnabled = true;
}

bool Video::Tick(u8 clockCycles, u8* pFrameBuffer)
{
    m_pFrameBuffer = pFrameBuffer;

    bool vblank = false;

    if (m_bScreenEnabled)
    {
        m_iStatusModeCounter += clockCycles;

        switch (m_iStatusMode)
        {
            case 0:
            {
                if (m_iStatusModeCounter >= 204)
                {
                    m_iStatusModeCounter -= 204;
                    m_iStatusMode = 2;

                    ScanLine(m_iStatusModeLYCounter);
                    m_iStatusModeLYCounter++;

                    UpdateLYRegister();

                    if (m_iStatusModeLYCounter == 144)
                    {
                        m_iStatusModeCounter = 0;
                        m_iStatusMode = 1;

                        m_pProcessor->RequestInterrupt(Processor::VBlank_Interrupt);
                        u8 stat = m_pMemory->Retrieve(0xFF41);
                        if (IsSetBit(stat, 4))
                            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);

                        vblank = true;
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
                m_iStatusModeCounterAux += clockCycles;

                if (m_iStatusModeCounterAux >= 456)
                {
                    m_iStatusModeLYCounter++;
                    m_iStatusModeCounterAux = 0;
                    UpdateLYRegister();
                }

                if (m_iStatusModeCounter >= 4560)
                {
                    m_iStatusModeCounter = 0;
                    m_iStatusModeCounterAux = 0;
                    m_iStatusMode = 2;
                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 5))
                        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
                    m_iStatusModeLYCounter = 0;
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

                    u8 stat = m_pMemory->Retrieve(0xFF41);
                    if (IsSetBit(stat, 3))
                        m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);

                    UpdateStatRegister();
                }
                break;
            }
        }
    }
    return vblank;
}

void Video::EnableScreen()
{
    if (!m_bScreenEnabled)
    {
        m_bScreenEnabled = true;
        u8 stat = m_pMemory->Retrieve(0xFF41);
        stat &= 0x78;
        m_pMemory->Load(0xFF41, stat);
        m_iStatusMode = 2;
        m_iStatusModeCounter = 0;
        m_iStatusModeCounterAux = 0;
        m_iStatusModeLYCounter = 0;
        UpdateLYRegister();
    }
}

void Video::DisableScreen()
{
    if (m_bScreenEnabled)
    {
        m_bScreenEnabled = false;
        u8 stat = m_pMemory->Retrieve(0xFF41);
        stat &= 0x78;
        m_pMemory->Load(0xFF41, stat);
        m_iStatusMode = 0;
        m_iStatusModeCounter = 0;
        m_iStatusModeCounterAux = 0;
        m_iStatusModeLYCounter = 0;
    }
}

bool Video::IsScreenEnabled()
{
    return m_bScreenEnabled;
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

    if (IsSetBit(lcdc, 0))
    {
        int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map = IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800;
        u8 scx = m_pMemory->Retrieve(0xFF43);
        u8 scy = m_pMemory->Retrieve(0xFF42);
        u8 lineAdjusted = line + scy;
        int y_32 = (lineAdjusted / 8) * 32;
        int pixely_2 = (lineAdjusted % 8) * 2;

        for (int x = 0; x < 32; x++)
        {
            int tile = 0;

            if (tiles == 0x8800)
            {
                tile = static_cast<s8> (m_pMemory->Retrieve(map + y_32 + x));
                tile += 128;
            }
            else
                tile = m_pMemory->Retrieve(map + y_32 + x);

            int mapOffsetX = x * 8;
            int tile_16 = tile * 16;
            u8 byte1 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2);
            u8 byte2 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2 + 1);

            for (int pixelx = 0; pixelx < 8; pixelx++)
            {
                u8 bufferX = (mapOffsetX + pixelx - scx);

                if (bufferX < GAMEBOY_WIDTH)
                {
                    int pixel = (byte1 & (0x1 << (7 - pixelx))) ? 1 : 0;
                    pixel |= (byte2 & (0x1 << (7 - pixelx))) ? 2 : 0;
                    u8 palette = m_pMemory->Retrieve(0xFF47);
                    u8 color = (palette >> (pixel * 2)) & 0x03;
                    m_pFrameBuffer[(line * GAMEBOY_WIDTH) + bufferX] = color;
                }
            }
        }
    }
    else
    {
        for (int x = 0; x < GAMEBOY_WIDTH; x++)
            m_pFrameBuffer[(line * GAMEBOY_WIDTH) + x] = 0;
    }
}

void Video::RenderWindow(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (IsSetBit(lcdc, 5))
    {
        u8 wy = m_pMemory->Retrieve(0xFF4A);

        if (wy <= line)
        {
            int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
            int map = IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800;
            u8 lineAdjusted = line - wy;
            int y_32 = (lineAdjusted / 8) * 32;
            int pixely_2 = (lineAdjusted % 8) * 2;
            int wx = m_pMemory->Retrieve(0xFF4B) - 7;

            for (int x = 0; x < 32; x++)
            {
                int tile = 0;

                if (tiles == 0x8800)
                {
                    tile = static_cast<s8> (m_pMemory->Retrieve(map + y_32 + x));
                    tile += 128;
                }
                else
                    tile = m_pMemory->Retrieve(map + y_32 + x);

                int mapOffsetX = x * 8;
                int tile_16 = tile * 16;
                u8 byte1 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2);
                u8 byte2 = m_pMemory->Retrieve(tiles + tile_16 + pixely_2 + 1);

                for (int pixelx = 0; pixelx < 8; pixelx++)
                {
                    int bufferX = (mapOffsetX + pixelx + wx);

                    if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                    {
                        int pixel = (byte1 & (0x1 << (7 - pixelx))) ? 1 : 0;
                        pixel |= (byte2 & (0x1 << (7 - pixelx))) ? 2 : 0;
                        u8 palette = m_pMemory->Retrieve(0xFF47);
                        u8 color = (palette >> (pixel * 2)) & 0x03;
                        m_pFrameBuffer[(line * GAMEBOY_WIDTH) + bufferX] = color;
                    }
                }
            }
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
                    int sprite_tile_16 = m_pMemory->Retrieve(0xFE00 + sprite_4 + 2) * 16;
                    u8 sprite_flags = m_pMemory->Retrieve(0xFE00 + sprite_4 + 3);
                    int sprite_pallette = IsSetBit(sprite_flags, 4) ? 1 : 0;
                    bool xflip = IsSetBit(sprite_flags, 5);
                    bool yflip = IsSetBit(sprite_flags, 6);
                    bool aboveBG = !IsSetBit(sprite_flags, 7);
                    int tiles = 0x8000;
                    int pixely_2 = (yflip ? sprite_height - 1 - (line - sprite_y) : line - sprite_y) * 2;

                    u8 byte1 = m_pMemory->Retrieve(tiles + sprite_tile_16 + pixely_2);
                    u8 byte2 = m_pMemory->Retrieve(tiles + sprite_tile_16 + pixely_2 + 1);

                    for (int pixelx = 0; pixelx < 8; pixelx++)
                    {
                        int pixel = (byte1 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 1 : 0;
                        pixel |= (byte2 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 2 : 0;

                        u8 palette = m_pMemory->Retrieve(sprite_pallette ? 0xFF49 : 0xFF48);
                        u8 color = (palette >> (pixel * 2)) & 0x03;

                        if (pixel)
                        {
                            int bufferX = (sprite_x + pixelx);

                            if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                            {
                                if (aboveBG)
                                {
                                    m_pFrameBuffer[(line * GAMEBOY_WIDTH) + bufferX] = color;
                                }
                                else if (m_pFrameBuffer[(line * GAMEBOY_WIDTH) + bufferX] == 0)
                                {
                                    m_pFrameBuffer[(line * GAMEBOY_WIDTH) + bufferX] = color;
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
    //if (m_bScreenEnabled)
    {
        // Updates the STAT register with current mode
        u8 stat = m_pMemory->Retrieve(0xFF41);
        m_pMemory->Load(0xFF41, (stat & 0xFC) | (m_iStatusMode & 0x3));
    }
}

void Video::UpdateLYRegister()
{
    //if (m_bScreenEnabled)
    {
        // Establish the LY register
        m_pMemory->Load(0xFF44, m_iStatusModeLYCounter);

        u8 lyc = m_pMemory->Retrieve(0xFF45);
        u8 stat = m_pMemory->Retrieve(0xFF41);

        if (lyc == m_iStatusModeLYCounter)
        {
            SetBit(stat, 2);
            if (IsSetBit(stat, 6))
                m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
        }
        else
            UnsetBit(stat, 2);

        m_pMemory->Load(0xFF41, stat);
    }
}
