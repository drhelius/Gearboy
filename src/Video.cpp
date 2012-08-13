#include "Video.h"
#include "Memory.h"
#include "Processor.h"

Video::Video(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    InitPointer(m_pFrameBuffer);
    InitPointer(m_pOffscreenBuffer);
    m_iStatusMode = 0;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_byStatusModeLYCounter = 0;
}

Video::~Video()
{
    SafeDeleteArray(m_pFrameBuffer);
    SafeDeleteArray(m_pOffscreenBuffer);
}

void Video::Init()
{
    m_pFrameBuffer = new u8[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    m_pOffscreenBuffer = new u8[256 * 256];

    Reset();
}

void Video::Reset()
{
    m_iStatusMode = 1;
    m_iStatusModeCounter = 0;
    m_iStatusModeCounterAux = 0;
    m_byStatusModeLYCounter = 144;

    for (int i = 0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        m_pFrameBuffer[i] = 0;
    for (int i = 0; i < (256 * 256); i++)
        m_pOffscreenBuffer[0] = 0;
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
                m_byStatusModeLYCounter++;
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

                u8 stat = m_pMemory->Retrieve(0xFF41);
                if (IsSetBit(stat, 3))
                    m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);

                UpdateStatRegister();
            }
            break;
        }
    }
    return vblank;
}

void Video::ScanLine(int line)
{
    if (line < GAMEBOY_HEIGHT)
    {
        RenderToOffscreen(line);


        for (int x = 0; x < GAMEBOY_WIDTH; x++)
        {
            m_pFrameBuffer[(line * GAMEBOY_WIDTH) + x] = m_pOffscreenBuffer[(line * 256) + x];
        }

    }
    /*
        u8 lcdc = m_pMemory->Retrieve(0xFF40);
        int tiles = (lcdc & 0x10) ? 0x8000 : 0x8800;
        int map = (lcdc & 0x8) ? 0x9C00 : 0x9800;

        int y = line / 8;

        for (int x = 0; x < 32; x++)
        {
            int tile = 0;

            if (tiles == 0x8800)
            {
                tile = static_cast<s8> (m_pMemory->Retrieve(map + ((y * 32) + x)));
                tile += 128;
            }
            else
            {
                tile = m_pMemory->Retrieve(map + ((y * 32) + x));
            }

            int offsetX = x * 8;
            int offsetY = y * 8;

            int h = line % 8;

            u8 byte1 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * h));
            u8 byte2 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * h) + 1);

            for (int w = 0; w < 8; w++)
            {
                int pixel = (byte1 & (0x1 << (7 - w))) ? 1 : 0;

                pixel |= (byte2 & (0x1 << (7 - w))) ? 2 : 0;

                int bufferX = (w + offsetX + m_pMemory->Retrieve(0xFF43)) % 256;
                int bufferY = (h + offsetY + m_pMemory->Retrieve(0xFF42)) % 256;

                m_pFrameBuffer[(bufferY * 256) + bufferX] = pixel;
            }

        }
     */
}

void Video::RenderToOffscreen(int line)
{
    RenderBG(line);
    RenderWindow(line);
    RenderSprites(line);
}

void Video::RenderBG(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (IsSetBit(lcdc, 0))
    {
        int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map = IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800;
        u8 scy = m_pMemory->Retrieve(0xFF42);
        u8 scx = m_pMemory->Retrieve(0xFF43);

        u8 lineAdjusted = line + scy;

        int y = lineAdjusted / 8;

        for (int x = 0; x < 32; x++)
        {
            int tile = 0;

            if (tiles == 0x8800)
            {
                tile = static_cast<s8> (m_pMemory->Retrieve(map + ((y * 32) + x)));
                tile += 128;
            }
            else
            {
                tile = m_pMemory->Retrieve(map + ((y * 32) + x));
            }

            int mapOffsetX = x * 8;
            int mapOffsetY = y * 8;

            int pixely = lineAdjusted % 8;

            u8 byte1 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * pixely));
            u8 byte2 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * pixely) + 1);

            for (int pixelx = 0; pixelx < 8; pixelx++)
            {
                int pixel = (byte1 & (0x1 << (7 - pixelx))) ? 1 : 0;

                pixel |= (byte2 & (0x1 << (7 - pixelx))) ? 2 : 0;

                u8 bufferX = (mapOffsetX + pixelx - scx);

                m_pOffscreenBuffer[(line * 256) + bufferX] = pixel;
            }
        }
    }
    else
    {
        for (int x = 0; x < GAMEBOY_WIDTH; x++)
        {
            m_pOffscreenBuffer[(line * 256) + x] = 0;
        }
    }
}

void Video::RenderWindow(int line)
{
    u8 lcdc = m_pMemory->Retrieve(0xFF40);

    if (IsSetBit(lcdc, 5))
    {
        int tiles = IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map = IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800;
        u8 wy = m_pMemory->Retrieve(0xFF4A);

        if (wy <= line)
        {
            u8 wx = m_pMemory->Retrieve(0xFF4B);

            u8 lineAdjusted = line - wy;

            int y = lineAdjusted / 8;

            for (int x = 0; x < 32; x++)
            {
                int tile = 0;

                if (tiles == 0x8800)
                {
                    tile = static_cast<s8> (m_pMemory->Retrieve(map + ((y * 32) + x)));
                    tile += 128;
                }
                else
                {
                    tile = m_pMemory->Retrieve(map + ((y * 32) + x));
                }

                int mapOffsetX = x * 8;
                int mapOffsetY = y * 8;

                int pixely = lineAdjusted % 8;

                u8 byte1 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * pixely));
                u8 byte2 = m_pMemory->Retrieve(tiles + (tile * 16) + (2 * pixely) + 1);

                for (int pixelx = 0; pixelx < 8; pixelx++)
                {
                    int pixel = (byte1 & (0x1 << (7 - pixelx))) ? 1 : 0;

                    pixel |= (byte2 & (0x1 << (7 - pixelx))) ? 2 : 0;

                    //if (pixel)
                    {
                        int bufferX = (mapOffsetX + pixelx + wx - 7);

                        if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                            m_pOffscreenBuffer[(line * 256) + bufferX] = pixel;
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
            int sprite_y = m_pMemory->Retrieve(0xFE00 + (4 * sprite)) - 16;

            if ((sprite_y <= line) && ((sprite_y + sprite_height) > line))
            {
                int sprite_x = m_pMemory->Retrieve(0xFE00 + (4 * sprite) + 1) - 8;

                if (sprite_x >= -7 && sprite_x < GAMEBOY_WIDTH)
                {
                    u8 sprite_tile = m_pMemory->Retrieve(0xFE00 + (4 * sprite) + 2);
                    u8 sprite_flags = m_pMemory->Retrieve(0xFE00 + (4 * sprite) + 3);
                    int sprite_pallette = IsSetBit(sprite_flags, 4) ? 1 : 0;
                    bool xflip = IsSetBit(sprite_flags, 5);
                    bool yflip = IsSetBit(sprite_flags, 6);
                    bool aboveBG = !IsSetBit(sprite_flags, 7);
                    int tiles = 0x8000;
                    int pixely = line - sprite_y;
                    if (yflip)
                        pixely = sprite_height - 1 - pixely;

                    u8 byte1 = m_pMemory->Retrieve(tiles + (sprite_tile * 16) + (2 * pixely));
                    u8 byte2 = m_pMemory->Retrieve(tiles + (sprite_tile * 16) + (2 * pixely) + 1);
                    for (int pixelx = 0; pixelx < 8; pixelx++)
                    {
                        int pixel = (byte1 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 1 : 0;

                        pixel |= (byte2 & (0x1 << (xflip ? pixelx : 7 - pixelx))) ? 2 : 0;

                        if (pixel)
                        {
                            int bufferX = (sprite_x + pixelx);

                            if (bufferX >= 0 && bufferX < GAMEBOY_WIDTH)
                            {
                                if (aboveBG)
                                {
                                    m_pOffscreenBuffer[(line * 256) + bufferX] = pixel;
                                }
                                else if (m_pOffscreenBuffer[(line * 256) + bufferX] == 0)
                                {
                                    m_pOffscreenBuffer[(line * 256) + bufferX] = pixel;
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

void Video::UpdateLYRegister()
{
    // Establish the LY register
    m_pMemory->Load(0xFF44, m_byStatusModeLYCounter);

    u8 lyc = m_pMemory->Retrieve(0xFF45);
    u8 stat = m_pMemory->Retrieve(0xFF41);

    if (lyc == m_byStatusModeLYCounter)
    {
        SetBit(stat, 2);
        if (IsSetBit(stat, 6))
            m_pProcessor->RequestInterrupt(Processor::LCDSTAT_Interrupt);
    }
    else
        UnsetBit(stat, 2);

    m_pMemory->Load(0xFF41, stat);
}
