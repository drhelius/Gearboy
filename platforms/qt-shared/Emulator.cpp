#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearboyCore);
}

Emulator::~Emulator()
{
    SafeDelete(m_pGearboyCore);
}

void Emulator::Init()
{
    m_pGearboyCore = new GearboyCore();
    m_pGearboyCore->Init();
}

void Emulator::LoadRom(const char* szFilePath)
{
    m_Mutex.lock();
    m_pGearboyCore->LoadROM(szFilePath);
    m_Mutex.unlock();
}

void Emulator::RunToVBlank(GB_Color* pFrameBuffer)
{
    m_Mutex.lock();
    m_pGearboyCore->RunToVBlank(pFrameBuffer);
    m_Mutex.unlock();
}

void Emulator::Pause()
{

}

void Emulator::Resume()
{

}
