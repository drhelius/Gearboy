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

Audio::Audio()
{
    m_bCGB = false;
    m_Time = 0;
    m_SampleRate = 44100;
    m_SampleCount = 0;
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSampleBuffer);
}

Audio::~Audio()
{
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDeleteArray(m_pSampleBuffer);
}

void Audio::Init()
{
    m_pSampleBuffer = new blip_sample_t[kSampleBufferSize];

    m_pApu = new Gb_Apu();
    m_pBuffer = new Stereo_Buffer();

    m_pBuffer->clock_rate(4194304);
    m_pBuffer->set_sample_rate(m_SampleRate);

    m_pApu->treble_eq(-15.0);
    m_pBuffer->bass_freq(100);

    m_pApu->set_output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());
}

void Audio::Reset(bool bCGB)
{
    m_bCGB = bCGB;

    Gb_Apu::mode_t mode = m_bCGB ? Gb_Apu::mode_cgb : Gb_Apu::mode_dmg;
    m_pApu->reset(mode);
    m_pBuffer->clear();

    for (int reg = 0xFF10; reg <= 0xFF3F; reg++)
    {
        u8 value = m_bCGB ? kInitialValuesForColorFFXX[reg - 0xFF00] : kInitialValuesForFFXX[reg - 0xFF00];
        m_pApu->write_register(0, reg, value);
    }

    m_Time = 0;
    m_SampleCount = 0;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_SampleRate)
    {
        m_SampleRate = rate;
        m_pBuffer->set_sample_rate(m_SampleRate);
    }
}

void Audio::EndFrame(short** pBuffer, long *pCount)
{
    m_pApu->end_frame(m_Time);
    m_pBuffer->end_frame(m_Time);
    m_SampleCount = m_pBuffer->read_samples(m_pSampleBuffer, kSampleBufferSize);
    m_Time = 0;
    *pBuffer = m_pSampleBuffer;
    *pCount = m_SampleCount;
}
