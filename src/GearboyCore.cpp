#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "IORegistersMemoryRule.h"
#include "RomOnlyMemoryRule.h"
#include "MBC1MemoryRule.h"
#include "MBC2MemoryRule.h"
#include "MBC3MemoryRule.h"

GearboyCore::GearboyCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pAudio);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pIORegistersMemoryRule);
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pMBC1MemoryRule);
    InitPointer(m_pMBC2MemoryRule);
    InitPointer(m_pMBC3MemoryRule);
    m_MBC = MBC_NONE;
}

GearboyCore::~GearboyCore()
{
    SafeDelete(m_pMBC3MemoryRule);
    SafeDelete(m_pMBC2MemoryRule);
    SafeDelete(m_pMBC1MemoryRule);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pIORegistersMemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pAudio);
    SafeDelete(m_pVideo);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearboyCore::Init()
{
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pAudio = new Audio();
    m_pInput = new Input(m_pMemory, m_pProcessor);
    m_pCartridge = new Cartridge();

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pVideo->Init();
    m_pAudio->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    InitMemoryRules();
}

void GearboyCore::Reset()
{
    m_MBC = MBC_NONE;
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pVideo->Reset();
    m_pAudio->Reset();
    m_pInput->Reset();
    m_pCartridge->Reset();

    m_pMBC1MemoryRule->Reset();
    m_pMBC2MemoryRule->Reset();
}

void GearboyCore::RunToVBlank(u8* pFrameBuffer)
{
    bool vblank = false;
    while (!vblank)
    {
        u8 clockCycles = m_pProcessor->Tick();
        vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
    }
}

void GearboyCore::LoadROM(const char* szFilePath)
{
    Reset();
    m_pCartridge->LoadFromFile(szFilePath);
    m_pMemory->LoadBank0and1FromROM(m_pCartridge->GetTheROM());
    AddMemoryRules();
}

Memory* GearboyCore::GetMemory()
{
    return m_pMemory;
}

void GearboyCore::InitMemoryRules()
{
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pIORegistersMemoryRule->SetMinAddress(0xFF00);
    m_pIORegistersMemoryRule->SetMaxAddress(0xFFFF);
    m_pIORegistersMemoryRule->Enable();

    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pRomOnlyMemoryRule->Enable();
    m_pRomOnlyMemoryRule->SetMinAddress(0x0000);
    m_pRomOnlyMemoryRule->SetMaxAddress(0xFEFF);

    m_pMBC1MemoryRule = new MBC1MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC1MemoryRule->Enable();
    m_pMBC1MemoryRule->SetMinAddress(0x0000);
    m_pMBC1MemoryRule->SetMaxAddress(0xFEFF);
    
    m_pMBC2MemoryRule = new MBC2MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC2MemoryRule->Enable();
    m_pMBC2MemoryRule->SetMinAddress(0x0000);
    m_pMBC2MemoryRule->SetMaxAddress(0xFEFF);
    
    m_pMBC3MemoryRule = new MBC3MemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput, m_pCartridge);
    m_pMBC3MemoryRule->Enable();
    m_pMBC3MemoryRule->SetMinAddress(0x0000);
    m_pMBC3MemoryRule->SetMaxAddress(0xFEFF);
}

void GearboyCore::AddMemoryRules()
{
    m_pMemory->AddRule(m_pIORegistersMemoryRule);
    
    int type = m_pCartridge->GetType();
    if (m_pCartridge->GetROMSize() == 0)
        type = 0;

    switch (type)
    {
        case 0x00:
            // NO MBC
            m_pMemory->AddRule(m_pRomOnlyMemoryRule);
            break;
        case 0x01:
            m_pMemory->AddRule(m_pMBC1MemoryRule);
            //this.cMBC1 = true;
            //MBCType = "MBC1";
            break;
        case 0x02:
            m_pMemory->AddRule(m_pMBC1MemoryRule);
            //            this.cMBC1 = true;
            //            this.cSRAM = true;
            //            MBCType = "MBC1 + SRAM";
            break;
        case 0x03:
            m_pMemory->AddRule(m_pMBC1MemoryRule);
            //            this.cMBC1 = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "MBC1 + SRAM + BATT";
            break;
        case 0x05:
            m_pMemory->AddRule(m_pMBC2MemoryRule);
            //            this.cMBC2 = true;
            //            MBCType = "MBC2";
            break;
        case 0x06:
            m_pMemory->AddRule(m_pMBC2MemoryRule);
            //            this.cMBC2 = true;
            //            this.cBATT = true;
            //            MBCType = "MBC2 + BATT";
            break;
        case 0x08:
            m_pMemory->AddRule(m_pRomOnlyMemoryRule);
            //            this.cSRAM = true;
            //            MBCType = "ROM + SRAM";
            break;
        case 0x09:
            m_pMemory->AddRule(m_pRomOnlyMemoryRule);
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "ROM + SRAM + BATT";
            break;
        case 0x0B:
            //            this.cMMMO1 = true;
            //            MBCType = "MMMO1";
            break;
        case 0x0C:
            //            this.cMMMO1 = true;
            //            this.cSRAM = true;
            //            MBCType = "MMMO1 + SRAM";
            break;
        case 0x0D:
            //            this.cMMMO1 = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "MMMO1 + SRAM + BATT";
            break;
        case 0x0F:
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            //            this.cMBC3 = true;
            //            this.cTIMER = true;
            //            this.cBATT = true;
            //            MBCType = "MBC3 + TIMER + BATT";
            break;
        case 0x10:
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            //            this.cMBC3 = true;
            //            this.cTIMER = true;
            //            this.cBATT = true;
            //            this.cSRAM = true;
            //            MBCType = "MBC3 + TIMER + BATT + SRAM";
            break;
        case 0x11:
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            //            this.cMBC3 = true;
            //            MBCType = "MBC3";
            break;
        case 0x12:
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            //            this.cMBC3 = true;
            //            this.cSRAM = true;
            //            MBCType = "MBC3 + SRAM";
            break;
        case 0x13:
            m_pMemory->AddRule(m_pMBC3MemoryRule);
            //            this.cMBC3 = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "MBC3 + SRAM + BATT";
            break;
        case 0x19:
            //            this.cMBC5 = true;
            //            MBCType = "MBC5";
            break;
        case 0x1A:
            //            this.cMBC5 = true;
            //            this.cSRAM = true;
            //            MBCType = "MBC5 + SRAM";
            break;
        case 0x1B:
            //            this.cMBC5 = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "MBC5 + SRAM + BATT";
            break;
        case 0x1C:
            //            this.cRUMBLE = true;
            //            MBCType = "RUMBLE";
            break;
        case 0x1D:
            //            this.cRUMBLE = true;
            //            this.cSRAM = true;
            //            MBCType = "RUMBLE + SRAM";
            break;
        case 0x1E:
            //            this.cRUMBLE = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "RUMBLE + SRAM + BATT";
            break;
        case 0x1F:
            //this.cCamera = true;
            //            MBCType = "GameBoy Camera";
            break;
        case 0x22:
            //            this.cMBC7 = true;
            //            this.cSRAM = true;
            //            this.cBATT = true;
            //            MBCType = "MBC7 + SRAM + BATT";
            break;
        case 0xFD:
            //            this.cTAMA5 = true;
            //            MBCType = "TAMA5";
            break;
        case 0xFE:
            //            this.cHuC3 = true;
            //            MBCType = "HuC3";
            break;
        case 0xFF:
            //            this.cHuC1 = true;
            //            MBCType = "HuC1";
            break;
        default:
            Log("--> ** Unknown cartridge type: %d", type);

    }
}

void GearboyCore::KeyPressed(Gameboy_Keys key)
{
    m_pInput->KeyPressed(key);
}

void GearboyCore::KeyReleased(Gameboy_Keys key)
{
    m_pInput->KeyReleased(key);
}

