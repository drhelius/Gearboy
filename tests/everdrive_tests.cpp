#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "Cartridge.h"
#include "EverdriveX.h"
#include "VirtualFat.h"

bool g_mcp_stdio_mode = true;

static void Check(bool ok, const char* message)
{
    if (!ok)
    {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static u32 Get32(const u8* p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static void TestDetection()
{
    u8 rom[0x14000];
    memset(rom, 0, sizeof(rom));
    memcpy(rom + 0x134, "GBXOS", 5);
    memset(rom + 0x139, 0xFF, 6); // stock v1.06 title padding
    memcpy(rom + 0x13F, "KRKZ", 4);
    rom[0x143] = 0x80;
    Cartridge cart;
    cart.Init();
    Check(cart.LoadFromBuffer(rom, sizeof(rom)), "load stock-shaped OS image");
    Check(cart.GetType() == Cartridge::CartridgeEverdriveOS, "detect FF-padded OS title");
    memset(rom + 0x139, 0, 6); // replacement OS titles can be NUL-padded
    Check(cart.LoadFromBuffer(rom, sizeof(rom)), "load replacement OS image");
    Check(cart.GetType() == Cartridge::CartridgeEverdriveOS, "detect NUL-padded OS title");
    rom[0x13F] = 'X';
    Check(cart.LoadFromBuffer(rom, sizeof(rom)), "load unrelated ROM");
    Check(cart.GetType() == Cartridge::CartridgeNoMBC, "require manufacturer signature");
}

static void TestFat(const char* root)
{
    VirtualFat fat;
    Check(fat.Open(root), "open virtual disk");
    u8 sector[512], mirror[512];
    fat.ReadSector(0, sector);
    u32 partition = Get32(sector + 454);
    fat.ReadSector(partition, sector);
    Check(sector[16] == 2, "publish two FATs");
    u32 reserved = sector[14] | (sector[15] << 8);
    u32 fat_sectors = Get32(sector + 36);
    u32 data_start = partition + reserved + 2 * fat_sectors;
    fat.ReadSector(data_start, sector);
    Check(!memcmp(sector, "PROBE   BIN", 11), "root directory at two-FAT data offset");
    u32 file_cluster = (sector[26] | (sector[27] << 8)) | ((u32)(sector[20] | (sector[21] << 8)) << 16);
    fat.ReadSector(data_start + (file_cluster - 2) * 64, sector);
    Check(!memcmp(sector, "test data", 9), "read host file contents");
    u32 first_fat = partition + reserved;
    fat.ReadSector(first_fat, sector);
    fat.ReadSector(first_fat + fat_sectors, mirror);
    Check(!memcmp(sector, mirror, 512), "FAT copies initially match");
    // Change a free entry through the second copy and verify both read back.
    sector[400] = 0xFF; sector[401] = 0xFF; sector[402] = 0xFF; sector[403] = 0x0F;
    fat.WriteSector(first_fat + fat_sectors, sector);
    fat.ReadSector(first_fat, mirror);
    Check(Get32(mirror + 400) == 0x0FFFFFFF, "second FAT write updates shared chain");
    fat.ReadSector(first_fat + fat_sectors, sector);
    Check(!memcmp(sector, mirror, 512), "FAT copies match after write");
}

static u8 Transfer(EverdriveX& card, u8 data)
{
    Check(card.Write(EverdriveX::k_reg_spi_data, data), "SPI write handled");
    u8 value = 0;
    Check(card.Read(EverdriveX::k_reg_spi_data, &value), "SPI read handled");
    return value;
}

static u8 Command(EverdriveX& card, u8 cmd, u32 arg)
{
    Transfer(card, 0x40 | cmd);
    for (int shift = 24; shift >= 0; shift -= 8)
        Transfer(card, (arg >> shift) & 0xFF);
    Transfer(card, 0xFF);
    for (int i = 0; i < 16; i++)
    {
        u8 response = Transfer(card, 0xFF);
        if (response != 0xFF)
            return response;
    }
    return 0xFF;
}

static void TestReadStream(const char* root)
{
    EverdriveX card;
    card.SetEnabled(true);
    card.SetFolder(root);
    card.Reset(root);
    card.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
    card.Write(EverdriveX::k_reg_spi_ctrl, 0);
    Check(Command(card, 0, 0) == 1, "SD reset response");
    Check(Command(card, 55, 0) == 1, "SD app command response");
    Check(Command(card, 41, 0x40000000) == 0, "SD initialization response");
    Check(Command(card, 18, 0) == 0, "CMD18 accepted");
    // Stock GBCOS deselects after R1, then selects again to read the block.
    card.Write(EverdriveX::k_reg_spi_ctrl, 1);
    for (int i = 0; i < 8; i++)
        Check(Transfer(card, 0xFF) == 0xFF, "deselected card does not clock stream");
    card.Write(EverdriveX::k_reg_spi_ctrl, 0);
    u8 token = 0xFF;
    for (int i = 0; i < 8 && token == 0xFF; i++)
        token = Transfer(card, 0xFF);
    Check(token == 0xFE, "CMD18 survives chip-select toggle");
    u8 data[512];
    for (int i = 0; i < 512; i++) data[i] = Transfer(card, 0xFF);
    Check(data[510] == 0x55 && data[511] == 0xAA, "streamed MBR contents");
    Transfer(card, 0xFF); Transfer(card, 0xFF); // CRC
    Check(Command(card, 12, 0) == 0, "CMD12 stops stream");
    card.Write(EverdriveX::k_reg_spi_ctrl, 1);
    card.Write(EverdriveX::k_reg_spi_ctrl, 0);
    for (int i = 0; i < 520; i++)
        Check(Transfer(card, 0xFF) == 0xFF, "stopped stream stays stopped");

    Check(Command(card, 25, 1) == 0, "CMD25 accepted");
    card.Write(EverdriveX::k_reg_spi_ctrl, 1);
    card.Write(EverdriveX::k_reg_spi_ctrl, 0);
    for (int block = 0; block < 2; block++)
    {
        Transfer(card, 0xFC);
        for (int i = 0; i < 512; i++) Transfer(card, (u8)(i + block));
        Transfer(card, 0xFF); Transfer(card, 0xFF); // CRC
        Check((Transfer(card, 0xFF) & 0x1F) == 0x05, "multi-write block accepted");
        Check(Transfer(card, 0xFF) == 0, "write busy byte");
        Check(Transfer(card, 0xFF) == 0xFF, "write completes");
    }
    Transfer(card, 0xFD);
    Check(Transfer(card, 0xFF) == 0xFF, "multi-write stop token completes");
    Check(Command(card, 18, 1) == 0, "read back multi-write");
    for (int block = 0; block < 2; block++)
    {
        token = 0xFF;
        for (int i = 0; i < 8 && token == 0xFF; i++) token = Transfer(card, 0xFF);
        Check(token == 0xFE, "read-back block token");
        for (int i = 0; i < 512; i++)
            Check(Transfer(card, 0xFF) == (u8)(i + block), "multi-write sector data and increment");
        Transfer(card, 0xFF); Transfer(card, 0xFF);
    }
    Check(Command(card, 12, 0) == 0, "stop read-back stream");
    Check(Command(card, 24, 3) == 0, "single-block writes still supported");
    Transfer(card, 0xFE);
    for (int i = 0; i < 514; i++) Transfer(card, 0xFF);
    Check((Transfer(card, 0xFF) & 0x1F) == 0x05, "single-write block accepted");
    Transfer(card, 0xFF); Transfer(card, 0xFF);
    Check(Command(card, 0, 0) == 1, "single-write returns to command mode");
}

static void I2CStart(EverdriveX& card)
{
    card.Write(0xBD0C, 0x20);
    card.Write(0xBD0D, 0x20);
    card.Write(0xBD0C, 0x11);
}

static void I2CByte(EverdriveX& card, u8 value)
{
    card.Write(0xBD0D, value);
    u8 status = 0xFF;
    Check(card.Read(0xBD0C, &status) && status == 0, "I2C transfer acknowledged and not busy");
}

static void TestI2C(const char* root)
{
    EverdriveX card;
    card.SetEnabled(true);
    card.SetFolder(root);
    card.Reset(root);
    card.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
    for (int device = 0; device < 2; device++)
    {
        I2CStart(card);
        I2CByte(card, 0xA0 + device * 2);
        if (device == 0) I2CByte(card, 0x12);
        I2CByte(card, 0);
        for (int i = 0; i < 19; i++) I2CByte(card, (u8)(i + device));
        card.Write(0xBD0C, 0x30);
        card.Write(0xBD0D, 0x30);
        // Bus resets must retain EEPROM contents.
        card.Reset(root);
        card.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
        I2CStart(card);
        I2CByte(card, 0xA0 + device * 2);
        if (device == 0) I2CByte(card, 0x12);
        I2CByte(card, 0);
        I2CStart(card);
        I2CByte(card, 0xA1 + device * 2);
        card.Write(0xBD0C, 0x10);
        for (int i = 0; i < 19; i++)
        {
            I2CByte(card, 0xFF);
            u8 data = 0xFF;
            Check(card.Read(0xBD0D, &data) && data == (u8)(i + device), "EEPROM read-back preserves settings");
        }
        card.Write(0xBD0C, 0x11);
        I2CByte(card, 0xFF);
        card.Write(0xBD0C, 0x30);
    }
}

static void TestCalendar(const char* root)
{
    EverdriveX card;
    card.SetEnabled(true);
    card.SetFolder(root);
    card.Reset(root);
    card.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
    // Write a valid leap day, keeping away from midnight while the test runs.
    const u8 calendar[] = {0x00,0x34,0x12,0x03,0x29,0x02,0x24,0,0,0,0,0,0,0,0,0};
    I2CStart(card); I2CByte(card, 0xD0); I2CByte(card, 0);
    for (unsigned i = 0; i < sizeof(calendar); i++) I2CByte(card, calendar[i]);
    card.Write(0xBD0C, 0x30);
    card.Reset(root);
    card.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
    I2CStart(card); I2CByte(card, 0xD0); I2CByte(card, 0);
    I2CStart(card); I2CByte(card, 0xD1); card.Write(0xBD0C, 0x10);
    for (unsigned i = 0; i < sizeof(calendar); i++)
    {
        I2CByte(card, 0xFF);
        u8 data; Check(card.Read(0xBD0D, &data), "read calendar byte");
        if (i == 0) Check(data <= 0x10, "calendar advances after setting");
        else if (i != 3) Check(data == calendar[i], "calendar preserves BCD date, time and status");
    }
    card.Write(0xBD0C, 0x30);
    setenv("GEARBOY_RTC", "0", 1);
    EverdriveX absent;
    unsetenv("GEARBOY_RTC");
    absent.SetEnabled(true); absent.Write(EverdriveX::k_reg_key, EverdriveX::k_key_unlock);
    I2CStart(absent); absent.Write(0xBD0D, 0xD0);
    u8 ack; Check(absent.Read(0xBD0C, &ack) && ack == 1, "clock-less cart NACKs calendar address");
}

int main()
{
    char root[] = "/tmp/gearboy-everdrive-tests-XXXXXX";
    Check(mkdtemp(root) != NULL, "create temporary SD folder");
    char path[512];
    snprintf(path, sizeof(path), "%s/PROBE.BIN", root);
    FILE* file = fopen(path, "wb");
    Check(file != NULL, "create probe file");
    Check(fwrite("test data", 1, 9, file) == 9, "write probe file");
    fclose(file);
    TestDetection();
    TestFat(root);
    TestReadStream(root);
    TestI2C(root);
    TestCalendar(root);
    unlink(path);
    rmdir(root);
    puts("EverDrive tests passed");
    return 0;
}
