//
//  EverdriveX.cpp
//  by Nolan Baker (Q*Bert Reynolds)
//

#include "EverdriveX.h"
#include "log.h"
#include <cstdlib>
#include <cstring>

EverdriveX::EverdriveX()
{
    m_image = NULL;
    m_image_path[0] = 0;
    Reset(NULL);
}

EverdriveX::~EverdriveX()
{
    CloseImage();
}

void EverdriveX::Reset(const char* rom_directory)
{
    m_unlocked = false;
    m_ctrl = k_ctrl_ss;
    m_data = 0xFF;
    m_state = State_Command;
    m_idle = true;
    m_app_cmd = false;
    m_cmd_length = 0;
    m_response_length = 0;
    m_response_pos = 0;
    m_write_pos = 0;
    m_write_sector = 0;

    if (rom_directory == NULL)
        return;

    CloseImage();
    const char* env = getenv("GEARBOY_SD_IMAGE");
    if (env && env[0])
        snprintf(m_image_path, sizeof(m_image_path), "%s", env);
    else if (rom_directory[0])
        snprintf(m_image_path, sizeof(m_image_path), "%s/sdcard.img", rom_directory);
    else
        snprintf(m_image_path, sizeof(m_image_path), "sdcard.img");
}

bool EverdriveX::OpenImage()
{
    if (m_image)
        return true;
    m_image = fopen(m_image_path, "r+b");
    if (!m_image)
        Log("EverdriveX: can't open SD card image %s", m_image_path);
    return m_image != NULL;
}

void EverdriveX::CloseImage()
{
    if (m_image)
        fclose(m_image);
    m_image = NULL;
}

bool EverdriveX::Read(u16 address, u8* value)
{
    if (!m_unlocked)
        return false;

    switch (address)
    {
        case k_reg_spi_data:
            *value = m_data;
            if (m_ctrl & k_ctrl_auto_read)
                Exchange(0xFF);
            return true;
        case k_reg_spi_ctrl:
            *value = m_ctrl & 0x7F; // transfers finish instantly, never busy
            return true;
        default:
            return false;
    }
}

bool EverdriveX::Write(u16 address, u8 value)
{
    if (address == k_reg_key)
    {
        m_unlocked = (value == k_key_unlock);
        return true;
    }

    if (!m_unlocked)
        return false;

    switch (address)
    {
        case k_reg_spi_data:
            Exchange(value);
            return true;
        case k_reg_spi_ctrl:
            if ((value & k_ctrl_ss) && !(m_ctrl & k_ctrl_ss))
            {
                // deselected, drop anything left over
                m_cmd_length = 0;
                m_response_length = 0;
                m_response_pos = 0;
            }
            m_ctrl = value & 0x7F;
            return true;
        default:
            return false;
    }
}

u8 EverdriveX::Exchange(u8 mosi)
{
    u8 miso = 0xFF;

    if (!(m_ctrl & k_ctrl_ss))
    {
        if (m_response_pos < m_response_length)
            miso = m_response[m_response_pos++];

        if (m_state == State_Command)
            ReceiveCommandByte(mosi);
        else
            ReceiveWriteByte(mosi);
    }

    m_data = miso;
    return miso;
}

void EverdriveX::ReceiveCommandByte(u8 mosi)
{
    if (m_cmd_length == 0 && (mosi & 0xC0) != 0x40)
        return; // waiting for a start bit

    m_cmd[m_cmd_length++] = mosi;
    if (m_cmd_length == 6)
    {
        m_cmd_length = 0;
        RunCommand();
    }
}

void EverdriveX::Respond(const u8* bytes, int count)
{
    m_response[0] = 0xFF; // one byte before the response
    memcpy(m_response + 1, bytes, count);
    m_response_length = count + 1;
    m_response_pos = 0;
}

void EverdriveX::RunCommand()
{
    u8 index = m_cmd[0] & 0x3F;
    u32 arg = (m_cmd[1] << 24) | (m_cmd[2] << 16) | (m_cmd[3] << 8) | m_cmd[4];
    bool app_cmd = m_app_cmd;
    m_app_cmd = false;
    u8 r[520];
    r[0] = m_idle ? 0x01 : 0x00;

    switch (index)
    {
        case 0: // GO_IDLE_STATE
            m_idle = true;
            r[0] = 0x01;
            Respond(r, 1);
            break;
        case 1: // SEND_OP_COND (MMC)
            m_idle = false;
            r[0] = 0x00;
            Respond(r, 1);
            break;
        case 8: // SEND_IF_COND, echo voltage and check pattern
            r[1] = 0x00;
            r[2] = 0x00;
            r[3] = (arg >> 8) & 0x0F;
            r[4] = arg & 0xFF;
            Respond(r, 5);
            break;
        case 12: // STOP_TRANSMISSION
        case 16: // SET_BLOCKLEN
            Respond(r, 1);
            break;
        case 17: // READ_SINGLE_BLOCK, SDHC block address
        {
            if (!OpenImage())
            {
                r[0] |= 0x20; // address error
                Respond(r, 1);
                break;
            }
            memset(r + 2, 0, 512);
            fseek(m_image, (long)arg * 512, SEEK_SET);
            if (fread(r + 2, 1, 512, m_image) < 512)
                clearerr(m_image);
            r[1] = 0xFE; // data token
            r[514] = 0xFF; // CRC
            r[515] = 0xFF;
            Respond(r, 516);
            break;
        }
        case 24: // WRITE_BLOCK
            if (!OpenImage())
            {
                r[0] |= 0x20;
                Respond(r, 1);
                break;
            }
            m_write_sector = arg;
            m_state = State_WriteToken;
            Respond(r, 1);
            break;
        case 41: // SD_SEND_OP_COND, ready immediately
            if (!app_cmd)
            {
                r[0] |= 0x04;
                Respond(r, 1);
                break;
            }
            m_idle = false;
            r[0] = 0x00;
            Respond(r, 1);
            break;
        case 55: // APP_CMD
            m_app_cmd = true;
            Respond(r, 1);
            break;
        case 58: // READ_OCR, powered up, SDHC
            r[1] = 0xC0;
            r[2] = 0xFF;
            r[3] = 0x80;
            r[4] = 0x00;
            Respond(r, 5);
            break;
        default:
            r[0] |= 0x04; // illegal command
            Respond(r, 1);
            break;
    }
}

void EverdriveX::ReceiveWriteByte(u8 mosi)
{
    if (m_state == State_WriteToken)
    {
        if (mosi == 0xFE)
        {
            m_state = State_WriteData;
            m_write_pos = 0;
        }
        return;
    }

    if (m_write_pos < 512)
        m_sector[m_write_pos] = mosi;
    if (++m_write_pos < 514) // data and 2 CRC bytes
        return;

    m_state = State_Command;
    fseek(m_image, (long)m_write_sector * 512, SEEK_SET);
    bool ok = fwrite(m_sector, 1, 512, m_image) == 512;
    fflush(m_image);

    m_response[0] = ok ? 0x05 : 0x0D; // data accepted / write error
    m_response[1] = 0x00; // busy for a byte
    m_response_length = 2;
    m_response_pos = 0;
}
