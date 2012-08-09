#include "Core.h"
#include "Memory.h"
#include "Processor.h"
#include "Video.h"
#include "Audio.h"
#include "Cartridge.h"

Core::Core()
{
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video();
    m_pAudio =  new Audio();
    m_pCartridge = new Cartridge();
}

Core::~Core()
{
    SafeDelete(m_pCartridge);
    SafeDelete(m_pAudio);
    SafeDelete(m_pVideo);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void Core::Reset()
{
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pVideo->Reset();
    m_pAudio->Reset();
    m_pCartridge->Reset();
}

void Core::RunToVBlank(u8* pFrameBuffer)
{
    m_pProcessor->Tick();
    m_pVideo->Tick();
    
    memcpy(pFrameBuffer, m_pVideo->GetFrameBuffer(), 256 * 256);
}

