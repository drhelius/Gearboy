//
//  EverdriveOSMemoryRule.cpp
//  by Nolan Baker (Q*Bert Reynolds)
//

#include "EverdriveOSMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"
#include <cstring>

EverdriveOSMemoryRule::EverdriveOSMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    m_pOS = new u8[0x20000];
    m_pPSRAM = new u8[k_psram_size];
    m_pSave = new u8[k_save_size];
    memset(m_pSave, 0xFF, k_save_size);
    Reset(false);
}

EverdriveOSMemoryRule::~EverdriveOSMemoryRule()
{
    SafeDeleteArray(m_pOS);
    SafeDeleteArray(m_pPSRAM);
    SafeDeleteArray(m_pSave);
}

void EverdriveOSMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_bUnlocked = false;
    m_Block[0] = 0;
    m_Block[1] = 0;
    m_Config = 0x04; // what the bootloader leaves
    m_Mapper = 0;
    m_Page = 0x04;
    m_bLaunch = false;
    m_Masks[0] = m_Masks[1] = 0;
    memset(m_pPSRAM, 0xFF, k_psram_size);
    memset(m_pOS, 0xFF, 0x20000);
    u8* rom = m_pCartridge->GetTheROM();
    int size = m_pCartridge->GetTotalSize();
    if (rom && size > 0)
    {
        if (size > 0x20000)
            Log("EverdriveOS: the X7's bootloader only takes a 128KB GBCOS.BIN (a 256KB one got 16KB), using the first 128KB");
        memcpy(m_pOS, rom, size < 0x20000 ? size : 0x20000);
    }
    m_Everdrive.Reset(m_pCartridge->GetFileDirectory());
}

// the byte the $BE00 window shows
u8* EverdriveOSMemoryRule::WindowByte(u16 address)
{
    u32 offset = ((m_Block[1] << 8 | m_Block[0]) << 9) + (address - 0xBE00);
    if (m_Config & 0x02)
        return &m_pSave[offset & (k_save_size - 1)];
    return &m_pPSRAM[offset & (k_psram_size - 1)];
}

u8 EverdriveOSMemoryRule::RegisterRead(u16 address)
{
    if (!m_bUnlocked)
        return 0x00;
    u8 value;
    if (m_Everdrive.Read(address, &value))
        return value;
    switch (address)
    {
        case 0xBD02: return m_Block[0];
        case 0xBD03: return m_Block[1];
        case 0xBD06: return m_Config;
        case 0xBD07: return m_Mapper;
        case 0xBD08: return m_Page;
        default: return 0x00;
    }
}

u8 EverdriveOSMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
        return m_pOS[address];
    if (address >= 0xA000 && address < 0xC000)
    {
        if (address >= 0xBE00)
            return *WindowByte(address);
        if (address >= 0xBD00 && address < 0xBD10)
            return RegisterRead(address);
        return m_pOS[(m_Page << 13) + (address - 0xA000)];
    }
    return m_pMemory->Retrieve(address);
}

u8 EverdriveOSMemoryRule::DebugRead(u16 address)
{
    if (address >= 0xBD00 && address < 0xBD10)
        return 0x00; // reading $BD00 can clock the SD card
    return PerformRead(address);
}

void EverdriveOSMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        m_pOS[address] = value; // RAM, writing $2000 doesn't switch banks
        return;
    }
    if (address >= 0xA000 && address < 0xC000)
    {
        if (address >= 0xBE00)
        {
            *WindowByte(address) = value;
            return;
        }
        if (address >= 0xBD00 && address < 0xBD10)
        {
            if (address == EverdriveX::k_reg_key)
            {
                m_bUnlocked = (value == EverdriveX::k_key_unlock);
                m_Everdrive.Write(address, value);
                return;
            }
            if (!m_bUnlocked || m_Everdrive.Write(address, value))
                return;
            switch (address)
            {
                case 0xBD02: m_Block[0] = value; break;
                case 0xBD03: m_Block[1] = value; break;
                case 0xBD04: m_Masks[0] = value; break;
                case 0xBD05: m_Masks[1] = value; break;
                case 0xBD06: m_Config = value; break;
                case 0xBD07:
                    m_Mapper = value;
                    if (value)
                    {
                        Log("EverdriveOS: game launch, mapper %d, $BD04 %02X $BD05 %02X $BD06 %02X", value, m_Masks[0], m_Masks[1], m_Config);
                        m_bLaunch = true;
                    }
                    break;
                case 0xBD08: m_Page = value & 0x0F; break;
                default: break;
            }
            return;
        }
        m_pOS[(m_Page << 13) + (address - 0xA000)] = value;
        return;
    }
    m_pMemory->Load(address, value);
}

void EverdriveOSMemoryRule::SaveRam(std::ostream &file)
{
    file.write(reinterpret_cast<const char*>(m_pSave), k_save_size);
}

bool EverdriveOSMemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    if ((fileSize > 0) && (fileSize != (s32)k_save_size))
    {
        Log("EverdriveOS: save RAM file is %d bytes, expected %d", fileSize, k_save_size);
        return false;
    }
    file.read(reinterpret_cast<char*>(m_pSave), k_save_size);
    return true;
}

size_t EverdriveOSMemoryRule::GetRamSize()
{
    return k_save_size;
}

u8* EverdriveOSMemoryRule::GetRamBanks()
{
    return m_pSave;
}

u8* EverdriveOSMemoryRule::GetCurrentRamBank()
{
    return m_pOS + (m_Page << 13);
}

int EverdriveOSMemoryRule::GetCurrentRamBankIndex()
{
    return m_Page;
}

u8* EverdriveOSMemoryRule::GetRomBank0()
{
    return m_pOS;
}

int EverdriveOSMemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* EverdriveOSMemoryRule::GetCurrentRomBank1()
{
    return m_pOS + 0x4000;
}

int EverdriveOSMemoryRule::GetCurrentRomBank1Index()
{
    return 1;
}

void EverdriveOSMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(m_pOS), 0x20000); // the image, PSRAM isn't saved
    stream.write(reinterpret_cast<const char*>(m_pSave), k_save_size);
    stream.write(reinterpret_cast<const char*>(&m_bUnlocked), sizeof(m_bUnlocked));
    stream.write(reinterpret_cast<const char*>(m_Block), sizeof(m_Block));
    stream.write(reinterpret_cast<const char*>(&m_Config), sizeof(m_Config));
    stream.write(reinterpret_cast<const char*>(&m_Mapper), sizeof(m_Mapper));
    stream.write(reinterpret_cast<const char*>(&m_Page), sizeof(m_Page));
}

void EverdriveOSMemoryRule::LoadState(std::istream& stream, u32)
{
    stream.read(reinterpret_cast<char*>(m_pOS), 0x20000);
    stream.read(reinterpret_cast<char*>(m_pSave), k_save_size);
    stream.read(reinterpret_cast<char*>(&m_bUnlocked), sizeof(m_bUnlocked));
    stream.read(reinterpret_cast<char*>(m_Block), sizeof(m_Block));
    stream.read(reinterpret_cast<char*>(&m_Config), sizeof(m_Config));
    stream.read(reinterpret_cast<char*>(&m_Mapper), sizeof(m_Mapper));
    stream.read(reinterpret_cast<char*>(&m_Page), sizeof(m_Page));
}

void EverdriveOSMemoryRule::SetEverdriveEnabled(bool enabled)
{
    m_Everdrive.SetEnabled(enabled);
}

void EverdriveOSMemoryRule::SetEverdriveFolder(const char* folder)
{
    m_Everdrive.SetFolder(folder);
}

bool EverdriveOSMemoryRule::LaunchRequested()
{
    return m_bLaunch;
}

// the game in PSRAM (its size from the header) and the save RAM, for the launch
void EverdriveOSMemoryRule::TakeLaunch(std::vector<u8>& rom, std::vector<u8>& save)
{
    m_bLaunch = false;
    u8 size_code = m_pPSRAM[0x148];
    u32 size = size_code <= 8 ? (0x8000u << size_code) : 0x8000;
    if (size > k_psram_size)
        size = k_psram_size;
    rom.assign(m_pPSRAM, m_pPSRAM + size);
    save.assign(m_pSave, m_pSave + k_save_size);
}
