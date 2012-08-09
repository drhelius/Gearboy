#include "GearboyCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Cartridge.h"

GearboyCore::GearboyCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pAudio);
    InitPointer(m_pCartridge);
}

GearboyCore::~GearboyCore()
{
    SafeDelete(m_pCartridge);
    SafeDelete(m_pAudio);
    SafeDelete(m_pVideo);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearboyCore::Init()
{
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video();
    m_pAudio = new Audio();
    m_pCartridge = new Cartridge();
    
    m_pMemory->Init();
    m_pProcessor->Init();
    m_pVideo->Init();
    m_pAudio->Init();
    m_pCartridge->Init();
}

void GearboyCore::Reset()
{
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pVideo->Reset();
    m_pAudio->Reset();
    m_pCartridge->Reset();
}

void GearboyCore::RunToVBlank(u8* pFrameBuffer)
{
    m_pProcessor->Tick();
    m_pVideo->Tick();

    memcpy(pFrameBuffer, m_pVideo->GetFrameBuffer(), 256 * 256);
}

