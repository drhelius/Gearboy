#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "IORegistersMemoryRule.h"

GearboyCore::GearboyCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pAudio);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pIORegistersMemoryRule);
}

GearboyCore::~GearboyCore()
{
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
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pVideo->Reset();
    m_pAudio->Reset();
    m_pInput->Reset();
    m_pCartridge->Reset();
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
    m_pMemory->LoadBank0FromROM(m_pCartridge->GetTheROM());
}

Memory* GearboyCore::GetMemory()
{
    return m_pMemory;
}

void GearboyCore::InitMemoryRules()
{
    m_pIORegistersMemoryRule = new IORegistersMemoryRule(m_pProcessor, m_pMemory,
            m_pVideo, m_pInput);
    m_pIORegistersMemoryRule->SetMinAddress(0xFF00);
    m_pIORegistersMemoryRule->SetMaxAddress(0xFFFF);
    m_pIORegistersMemoryRule->Enable();
    m_pMemory->AddRule(m_pIORegistersMemoryRule);
}

void GearboyCore::KeyPressed(Gameboy_Keys key)
{
    m_pInput->KeyPressed(key);
}

void GearboyCore::KeyReleased(Gameboy_Keys key)
{
    m_pInput->KeyReleased(key);
}

