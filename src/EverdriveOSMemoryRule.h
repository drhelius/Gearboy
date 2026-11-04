//
//  EverdriveOSMemoryRule.h
//  by Nolan Baker (Q*Bert Reynolds)
//
//  EverDrive GB X-series OS mode, for ROMs whose header says GBXOS/KRKZ (/GBCSYS/GBCOS.BIN on
//  the real cart). Measured on an X7 with OS v1.06:
//    - the 128KB image is RAM: $0000-$7FFF is pages 0-3 (writable, no bank switching), and
//      $A000-$BFFF shows page $BD08 (4 bits)
//    - after writing $A5 to $BD0A, $BD00-$BD0F are registers: $BD00/$BD01 SPI (EverdriveX),
//      $BD02/$BD03 the 512 byte block shown at $BE00-$BFFF, $BD06 bit 1 picks save RAM (set)
//      or PSRAM (clear) for that window, $BD07 mapper (a game launch: the cart resets the Game Boy
//      into the game in PSRAM from block 0 with $BD06 = $08, see GearboyCore::LaunchEverdriveGame),
//      $BD08 page. Locked, they read 0
//    - GBCOS.BIN has to be 128KB. The image is separate from PSRAM (the EverDrive's OS loads games
//      at PSRAM block 0 while it runs)
//

#ifndef EVERDRIVEOSMEMORYRULE_H
#define EVERDRIVEOSMEMORYRULE_H

#include "MemoryRule.h"
#include "EverdriveX.h"
#include <vector>

class EverdriveOSMemoryRule : public MemoryRule
{
public:
    static const u32 k_psram_size = 0x800000;
    static const u32 k_save_size = 0x20000;

    EverdriveOSMemoryRule(Memory* pMemory, Cartridge* pCartridge);
    virtual ~EverdriveOSMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual u8 DebugRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual int GetCurrentRamBankIndex();
    virtual u8* GetRomBank0();
    virtual int GetCurrentRomBank0Index();
    virtual u8* GetCurrentRomBank1();
    virtual int GetCurrentRomBank1Index();
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream, u32 version = GB_SAVESTATE_VERSION);
    void SetEverdriveEnabled(bool enabled);
    bool LaunchRequested();
    void TakeLaunch(std::vector<u8>& rom, std::vector<u8>& save);
    void SetEverdriveFolder(const char* folder);

private:
    u8* WindowByte(u16 address);
    u8 RegisterRead(u16 address);

private:
    u8* m_pOS; // the 128KB image
    u8* m_pPSRAM;
    bool m_bLaunch;
    u8* m_pSave;
    bool m_bUnlocked;
    u8 m_Block[2];
    u8 m_Masks[2]; // $BD04 ROM, $BD05 save RAM bank masks for a launched game
    u8 m_Config; // $BD06
    u8 m_Mapper; // $BD07
    u8 m_Page; // $BD08
    EverdriveX m_Everdrive;
};

#endif // EVERDRIVEOSMEMORYRULE_H
