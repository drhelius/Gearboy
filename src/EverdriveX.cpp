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
    m_i2c_memory[0].resize(0x10000, 0);
    m_i2c_memory[1].resize(0x100, 0);
    m_image = NULL;
    m_image_path[0] = 0;
    m_folder[0] = 0;
    m_root[0] = 0;
    const char* env = getenv("GEARBOY_EVERDRIVE");
    m_enabled = !(env && env[0] == '0');
    Reset(NULL);
}

EverdriveX::~EverdriveX()
{
    CloseCard();
}

void EverdriveX::Reset(const char* rom_directory)
{
    m_unlocked = false;
    m_i2c_state = I2C_Idle;
    m_i2c_address = 0;
    m_i2c_device = 0;
    m_i2c_address_bytes = 0;
    m_i2c_data = m_i2c_ack = 0;
    m_ctrl = k_ctrl_ss;
    m_data = 0xFF;
    m_state = State_Command;
    m_idle = true;
    m_app_cmd = false;
    m_cmd_length = 0;
    m_response_length = 0;
    m_multi_read = false;
    m_response_pos = 0;
    m_write_pos = 0;
    m_write_sector = 0;

    if (rom_directory == NULL)
        return;

    CloseCard();
    m_image_path[0] = 0;
    m_root[0] = 0;
    const char* image = getenv("GEARBOY_SD_IMAGE");
    const char* dir = getenv("GEARBOY_SD_DIR");
    const char* home = getenv("HOME");
    if (!home)
        home = getenv("USERPROFILE");
    if (image && image[0])
        snprintf(m_image_path, sizeof(m_image_path), "%s", image);
    else if (dir && dir[0])
        snprintf(m_root, sizeof(m_root), "%s", dir);
    else if (m_folder[0])
        snprintf(m_root, sizeof(m_root), "%s", m_folder);
    else if (home)
        snprintf(m_root, sizeof(m_root), "%s", home);
    else
        snprintf(m_image_path, sizeof(m_image_path), "%s/sdcard.img", rom_directory[0] ? rom_directory : ".");
}

void EverdriveX::SetFolder(const char* folder)
{
    snprintf(m_folder, sizeof(m_folder), "%s", folder ? folder : "");
}

void EverdriveX::SetEnabled(bool enabled)
{
    m_enabled = enabled;
    m_unlocked = false;
}

bool EverdriveX::IsEnabled() const
{
    return m_enabled;
}

bool EverdriveX::OpenCard()
{
    if (m_root[0])
        return m_fat.IsOpen() || m_fat.Open(m_root);
    if (m_image)
        return true;
    m_image = fopen(m_image_path, "r+b");
    if (!m_image)
        Log("EverdriveX: can't open SD card image %s", m_image_path);
    return m_image != NULL;
}

void EverdriveX::CloseCard()
{
    if (m_image)
        fclose(m_image);
    m_image = NULL;
    m_fat.Close();
}

bool EverdriveX::Read(u16 address, u8* value)
{
    if (!m_enabled || !m_unlocked)
        return false;

    switch (address)
    {
        case k_reg_spi_data:
            *value = m_data;
            if (m_ctrl & k_ctrl_auto_read)
                Exchange(0xFF);
            return true;
        case 0xBD0C:
            *value = m_i2c_ack; // byte transfers complete immediately, busy bit clear
            return true;
        case 0xBD0D:
            *value = m_i2c_data;
            return true;
        case k_reg_spi_ctrl:
            *value = m_ctrl & 0x0F; // only bits 0-3 read back (X7), transfers finish instantly, never busy
            return true;
        default:
            return false;
    }
}

bool EverdriveX::Write(u16 address, u8 value)
{
    if (!m_enabled)
        return false;

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
        case 0xBD0C:
            WriteI2CControl(value);
            return true;
        case 0xBD0D:
            WriteI2CData(value);
            return true;
        case k_reg_spi_ctrl:
            if ((value & k_ctrl_ss) && !(m_ctrl & k_ctrl_ss))
            {
                // CS pauses a CMD18 stream; CMD12 ends it. Stock GBCOS toggles
                // CS after receiving R1, before consuming the data blocks.
                m_cmd_length = 0;
                if (!m_multi_read)
                {
                    m_response_length = 0;
                    m_response_pos = 0;
                }
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
        if (m_multi_read && m_response_pos >= m_response_length)
            QueueBlock();
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

// the next block of a READ_MULTIPLE_BLOCK: a gap, the data token, the sector, its CRC
void EverdriveX::QueueBlock()
{
    m_response[0] = 0xFF;
    m_response[1] = 0xFE;
    memset(m_response + 2, 0, 512);
    if (m_fat.IsOpen())
        m_fat.ReadSector(m_multi_sector, m_response + 2);
    else
    {
        fseek(m_image, (long)m_multi_sector * 512, SEEK_SET);
        if (fread(m_response + 2, 1, 512, m_image) < 512)
            clearerr(m_image);
    }
    m_response[514] = 0xFF;
    m_response[515] = 0xFF;
    m_response_length = 516;
    m_response_pos = 0;
    m_multi_sector++;
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
        case 12: // STOP_TRANSMISSION (a stuff byte, then R1)
            m_multi_read = false;
            r[0] = 0x00;
            Respond(r, 1);
            break;
        case 18: // READ_MULTIPLE_BLOCK, blocks follow R1 until STOP_TRANSMISSION
            if (!OpenCard())
            {
                r[0] |= 0x20;
                Respond(r, 1);
                break;
            }
            m_multi_read = true;
            m_multi_sector = arg;
            Respond(r, 1);
            break;
        case 16: // SET_BLOCKLEN
            Respond(r, 1);
            break;
        case 17: // READ_SINGLE_BLOCK, SDHC block address
        {
            if (!OpenCard())
            {
                r[0] |= 0x20; // address error
                Respond(r, 1);
                break;
            }
            memset(r + 2, 0, 512);
            if (m_fat.IsOpen())
                m_fat.ReadSector(arg, r + 2);
            else
            {
                fseek(m_image, (long)arg * 512, SEEK_SET);
                if (fread(r + 2, 1, 512, m_image) < 512)
                    clearerr(m_image);
            }
            r[1] = 0xFE; // data token
            r[514] = 0xFF; // CRC
            r[515] = 0xFF;
            Respond(r, 516);
            break;
        }
        case 24: // WRITE_BLOCK
        case 25: // WRITE_MULTIPLE_BLOCK (used by stock GBCOS)
            if (!OpenCard())
            {
                r[0] |= 0x20;
                Respond(r, 1);
                break;
            }
            m_write_sector = arg;
            m_state = (index == 25) ? State_MultiWriteToken : State_WriteToken;
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
    bool multi_write = (m_state == State_MultiWriteToken || m_state == State_MultiWriteData);
    if (m_state == State_WriteToken || m_state == State_MultiWriteToken)
    {
        if (multi_write && mosi == 0xFD) // Stop Tran token
            m_state = State_Command;
        else if (mosi == (multi_write ? 0xFC : 0xFE))
        {
            m_state = multi_write ? State_MultiWriteData : State_WriteData;
            m_write_pos = 0;
        }
        return;
    }

    if (m_write_pos < 512)
        m_sector[m_write_pos] = mosi;
    if (++m_write_pos < 514) // data and 2 CRC bytes
        return;

    m_state = multi_write ? State_MultiWriteToken : State_Command;
    bool ok = true;
    if (m_fat.IsOpen())
        m_fat.WriteSector(m_write_sector, m_sector);
    else
    {
        fseek(m_image, (long)m_write_sector * 512, SEEK_SET);
        ok = fwrite(m_sector, 1, 512, m_image) == 512;
        fflush(m_image);
    }

    if (ok && multi_write)
        m_write_sector++;

    m_response[0] = ok ? 0x05 : 0x0D; // data accepted / write error
    m_response[1] = 0x00; // busy for a byte
    m_response_length = 2;
    m_response_pos = 0;
}

// GBCOS starts with C=$20, D=$20, C=$11, then sends the device/address
// through D. Repeated START selects reading without losing the address cursor.
// C=$10 enables ACK during reads; C=$11 ends them; C=$30 resets the bus.
void EverdriveX::WriteI2CControl(u8 value)
{
    if (value == 0x20)
    {
        m_i2c_state = I2C_Start;
        m_i2c_ack = 0;
    }
    else if (value == 0x30)
        m_i2c_state = I2C_Idle;
    else if (value == 0x11)
        m_i2c_state = (m_i2c_state == I2C_Start) ? I2C_Device : I2C_Idle;
}

void EverdriveX::WriteI2CData(u8 value)
{
    switch (m_i2c_state)
    {
        case I2C_Device:
            m_i2c_ack = ((value & 0xFC) == 0xA0) ? 0 : 1;
            if (m_i2c_ack)
            {
                m_i2c_state = I2C_Idle;
                break;
            }
            m_i2c_device = (value >> 1) & 1;
            if (value & 1)
                m_i2c_state = I2C_Read;
            else
            {
                m_i2c_address = 0;
                m_i2c_address_bytes = m_i2c_device ? 1 : 2;
                m_i2c_state = I2C_Address;
            }
            break;
        case I2C_Address:
            m_i2c_address = (m_i2c_address << 8) | value;
            if (--m_i2c_address_bytes == 0)
                m_i2c_state = I2C_Write;
            break;
        case I2C_Write:
            m_i2c_memory[m_i2c_device][m_i2c_address % m_i2c_memory[m_i2c_device].size()] = value;
            m_i2c_address++;
            break;
        case I2C_Read:
            m_i2c_data = m_i2c_memory[m_i2c_device][m_i2c_address % m_i2c_memory[m_i2c_device].size()];
            m_i2c_address++;
            break;
        default:
            break;
    }
}
