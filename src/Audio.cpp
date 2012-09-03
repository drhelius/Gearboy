/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/ 
 * 
 */

#include "Audio.h"
#include "Memory.h"
#include "audio/Sound_Queue.h"
#include "audio/Gb_Apu.h"

Audio::Audio()
{
    m_bCGB = false;
    m_bEnabled = true;
    m_Time = 0;
    m_iSampleRate = 44100;
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSound);
    InitPointer(m_pSampleBuffer);
}

Audio::~Audio()
{
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDelete(m_pSound);
    SafeDeleteArray(m_pSampleBuffer);
}

void Audio::Init()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        Log("--> ** SDL Audio not initialized");
    }

    atexit(SDL_Quit);

    m_pSampleBuffer = new blip_sample_t[kSampleBufferSize];

    m_pApu = new Gb_Apu();
    m_pBuffer = new Stereo_Buffer();
    m_pSound = new Sound_Queue();

    m_pBuffer->clock_rate(4194304);
    m_pBuffer->set_sample_rate(m_iSampleRate);

    // Adjust frequency equalization to make it sound like a tiny speaker
    m_pApu->treble_eq(-20.0); // lower values muffle it more
    m_pBuffer->bass_freq(461); // higher values simulate smaller speaker

    m_pApu->set_output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());

    m_pSound->start(m_iSampleRate, 2);
}

void Audio::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_bEnabled = true;
    Gb_Apu::mode_t mode = m_bCGB ? Gb_Apu::mode_cgb : Gb_Apu::mode_dmg;
    m_pApu->reset(mode);
    m_pBuffer->clear();
    
    for (int reg = 0xFF10; reg <= 0xFF3F; reg++)
        m_pApu->write_register(0, reg, kInitialValuesForFFXX[reg - 0xFF00]);
    
    m_Time = 0;
}

void Audio::Enable(bool enabled)
{
    m_bEnabled = enabled;
}

bool Audio::IsEnabled() const
{
    return m_bEnabled;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_iSampleRate)
    {
        m_iSampleRate = rate;
        m_pBuffer->set_sample_rate(m_iSampleRate);
        m_pSound->stop();
        m_pSound->start(m_iSampleRate, 2);
    }
}

u8 Audio::ReadAudioRegister(u16 address)
{
    return m_pApu->read_register(m_Time, address);
}

void Audio::WriteAudioRegister(u16 address, u8 value)
{
    m_pApu->write_register(m_Time, address, value);
}

void Audio::EndFrame()
{
    m_pApu->end_frame(kSoundFrameLength);
    m_pBuffer->end_frame(kSoundFrameLength);

    if (m_pBuffer->samples_avail() >= kSampleBufferSize)
    {
        long count = m_pBuffer->read_samples(m_pSampleBuffer, kSampleBufferSize);
        if (m_bEnabled)
        {
            m_pSound->write(m_pSampleBuffer, count);
        }
    }
}

void Audio::Tick(u8 clockCycles)
{
    m_Time += clockCycles;

    if (m_Time >= kSoundFrameLength)
    {
        m_Time -= kSoundFrameLength;

        EndFrame();
    }
}
