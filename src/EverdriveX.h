//
//  EverdriveX.h
//  by Nolan Baker (Q*Bert Reynolds)
//
//  EverDrive GB X-series SD card interface. Registers live in cartridge RAM
//  space and only respond after writing $A5 to the key register:
//    $BD00 SPI data, $BD01 SPI control (SS, low speed, auto read, busy), $BD0A key
//  The SD card is an SDHC card over SPI. Its files are a host folder (VirtualFat): the
//  configured one (SetFolder), GEARBOY_SD_DIR, or the home folder. GEARBOY_SD_IMAGE uses a
//  disk image instead. GEARBOY_EVERDRIVE=0 starts with the cart disabled.
//

#ifndef EVERDRIVE_X_H
#define EVERDRIVE_X_H

#include "definitions.h"
#include "VirtualFat.h"
#include <cstdio>
#include <ctime>
#include <vector>

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
    void SetEnabled(bool enabled);
    void SetFolder(const char* folder); // used from the next Reset, empty for the home folder
    bool IsEnabled() const;
    bool Read(u16 address, u8* value);
    bool Write(u16 address, u8 value);
    bool GetClockRegisters(u8* registers);
    u8 GetCartridgeFeatures() const; // last $BD0F feature mask written by the OS

private:
    u8 Exchange(u8 mosi);
    void ReceiveCommandByte(u8 mosi);
    void ReceiveWriteByte(u8 mosi);
    void RunCommand();
    void QueueBlock();
    void Respond(const u8* bytes, int count);
    bool OpenCard();
    void CloseCard();
    void WriteI2CControl(u8 value);
    void WriteI2CData(u8 value);
    void UpdateClockRegisters();
    void CommitClockRegisters();

private:
    enum State
    {
        State_Command,
        State_WriteToken,
        State_WriteData,
        State_MultiWriteToken,
        State_MultiWriteData
    };

    // Byte-level EEPROM transactions used by GBCOS ($BD0C/$BD0D).
    enum I2CState { I2C_Idle, I2C_Start, I2C_Device, I2C_Address, I2C_Write, I2C_Read };
    I2CState m_i2c_state;
    std::vector<u8> m_i2c_memory[3]; // $A0: EEPROM; $A2: settings; $D0: calendar RTC
    u16 m_i2c_address;
    u8 m_i2c_device;
    u8 m_i2c_address_bytes;
    u8 m_i2c_data;
    u8 m_i2c_ack;
    u8 m_cartridge_features;

    std::time_t m_clock_offset;
    bool m_clock_dirty;
    bool m_clock_present;

    bool m_enabled;
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
    bool m_multi_read; // READ_MULTIPLE_BLOCK is streaming m_multi_sector on
    u32 m_multi_sector;
    u8 m_sector[512];
    int m_write_pos;
    u32 m_write_sector;
    char m_image_path[600];
    char m_folder[600];
    char m_root[600];
    FILE* m_image;
    VirtualFat m_fat;
};

#endif // EVERDRIVE_X_H
