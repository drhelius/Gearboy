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
#include "audio/gb_apu/Gb_Apu.h"

gb_time_t const frame_length = 70224;

Audio::Audio()
{
    m_bEnabled = true;
    m_Time = 0;
    m_iSampleRate = 0;
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

void Audio::Init(int sampleRate)
{
    m_iSampleRate = sampleRate;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        Log("--> ** SDL Audio not initialized");
    }

    atexit(SDL_Quit);

    m_pSampleBuffer = new blip_sample_t[kSampleBufferSize];

    Reset();
}

void Audio::Reset()
{
    m_bEnabled = true;

    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDelete(m_pSound);
    m_pApu = new Gb_Apu();
    m_pBuffer = new Stereo_Buffer();
    m_pSound = new Sound_Queue();

    // Adjust frequency equalization to make it sound like a tiny speaker
    m_pApu->treble_eq(-20.0); // lower values muffle it more
    m_pBuffer->bass_freq(461); // higher values simulate smaller speaker

    m_pApu->output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());
    m_pBuffer->clock_rate(4194304);
    m_pBuffer->set_sample_rate(m_iSampleRate);
    m_pSound->start(m_iSampleRate, 2);

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

u8 Audio::ReadAudioRegister(u16 address)
{
    if (m_bEnabled)
    {
        m_Time += 4;
        return m_pApu->read_register(m_Time, address) | kSoundMask[address - 0xFF10];
    }
    else
        return kSoundMask[address - 0xFF10];
}

void Audio::WriteAudioRegister(u16 address, u8 value)
{
    if (m_bEnabled)
    {
        m_Time += 4;
        if ((address == 0xFF26) && ((value & 0x80) == 0))
        {
            for (int i = 0xFF10; i <= 0xFF26; i++)
                m_pApu->write_register(m_Time, i, 0);
        }
        else
        {
            if ((address >= 0xFF30) || (address == 0xFF26) || (address == 0xFF20) || (m_pApu->read_register(m_Time, 0xFF26) & 0x80))
                m_pApu->write_register(m_Time, address, value);
        }
    }
}


void Audio::EndFrame()
{
    m_Time = 0;

    if (m_bEnabled)
    {
        bool stereo = m_pApu->end_frame(frame_length);
        m_pBuffer->end_frame(frame_length, stereo);

        if (m_pBuffer->samples_avail() >= kSampleBufferSize)
        {
            long count = m_pBuffer->read_samples(m_pSampleBuffer, kSampleBufferSize);
            m_pSound->write(m_pSampleBuffer, count);
        }
    }
}
