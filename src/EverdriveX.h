//
//  EverdriveX.h
//  by Nolan Baker (Q*Bert Reynolds)
//
//  EverDrive GB X-series SD card interface. Registers live in cartridge RAM
//  space and only respond after writing $A5 to the key register:
//    $BD00 SPI data, $BD01 SPI control (SS, low speed, auto read, busy), $BD0A key
//  The SD card is an SDHC card over SPI backed by a disk image, sdcard.img next
//  to the ROM, or GEARBOY_SD_IMAGE.
//

#ifndef EVERDRIVE_X_H
#define EVERDRIVE_X_H

#include "definitions.h"
#include <cstdio>

class EverdriveX
{
public:
    static const u16 k_reg_spi_data = 0xBD00;
    static const u16 k_reg_spi_ctrl = 0xBD01;
    static const u16 k_reg_key = 0xBD0A;
    static const u8 k_key_unlock = 0xA5;
    static const u8 k_ctrl_ss = 0x01;
    static const u8 k_ctrl_auto_read = 0x04;

    EverdriveX();
    ~EverdriveX();
    void Reset(const char* rom_directory);
    bool Read(u16 address, u8* value);
    bool Write(u16 address, u8 value);

private:
    u8 Exchange(u8 mosi);
    void ReceiveCommandByte(u8 mosi);
    void ReceiveWriteByte(u8 mosi);
    void RunCommand();
    void Respond(const u8* bytes, int count);
    bool OpenImage();
    void CloseImage();

private:
    enum State
    {
        State_Command,
        State_WriteToken,
        State_WriteData
    };

    bool m_unlocked;
    u8 m_ctrl;
    u8 m_data;
    State m_state;
    bool m_idle;
    bool m_app_cmd;
    u8 m_cmd[6];
    int m_cmd_length;
    u8 m_response[600];
    int m_response_length;
    int m_response_pos;
    u8 m_sector[512];
    int m_write_pos;
    u32 m_write_sector;
    char m_image_path[600];
    FILE* m_image;
};

#endif // EVERDRIVE_X_H
