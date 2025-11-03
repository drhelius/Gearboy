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
    m_ElapsedCycles = 0;
    m_SampleRate = 44100;
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSampleBuffer);
    m_bVgmRecordingEnabled = false;
}

Audio::~Audio()
{
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDeleteArray(m_pSampleBuffer);
}

void Audio::Init()
{
    m_pSampleBuffer = new blip_sample_t[AUDIO_BUFFER_SIZE];

    m_pApu = new Gb_Apu();
    m_pBuffer = new Stereo_Buffer();

    m_pBuffer->clock_rate(4194304);
    m_pBuffer->set_sample_rate(m_SampleRate);

    //m_pApu->treble_eq(-15.0);
    //m_pBuffer->bass_freq(100);

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

    m_ElapsedCycles = 0;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_SampleRate)
    {
        m_SampleRate = rate;
        m_pBuffer->set_sample_rate(m_SampleRate);
    }
}

void Audio::SetVolume(float volume)
{
    m_pApu->volume(volume);
}

void Audio::EndFrame(s16* pSampleBuffer, int* pSampleCount)
{
    m_pApu->end_frame(m_ElapsedCycles);
    m_pBuffer->end_frame(m_ElapsedCycles);

    int count = static_cast<int>(m_pBuffer->read_samples(m_pSampleBuffer, AUDIO_BUFFER_SIZE));

    if (IsValidPointer(pSampleBuffer) && IsValidPointer(pSampleCount))
    {
        *pSampleCount = count;

        for (int i=0; i<count; i++)
        {
            pSampleBuffer[i] = m_pSampleBuffer[i];
        }
    }

#ifndef GEARBOY_DISABLE_VGMRECORDER
    if (m_bVgmRecordingEnabled)
        m_VgmRecorder.UpdateTiming(count / 2);
#endif

    m_ElapsedCycles = 0;
}

void Audio::SaveState(std::ostream& stream)
{
    using namespace std;

    gb_apu_state_t apu_state;

    m_pApu->save_state(&apu_state);

    stream.write(reinterpret_cast<const char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.write(reinterpret_cast<const char*> (m_pSampleBuffer), sizeof(blip_sample_t) * AUDIO_BUFFER_SIZE);
    stream.write(reinterpret_cast<const char*> (&apu_state), sizeof(apu_state));
}

void Audio::LoadState(std::istream& stream)
{
    using namespace std;

    gb_apu_state_t apu_state;

    stream.read(reinterpret_cast<char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.read(reinterpret_cast<char*> (m_pSampleBuffer), sizeof(blip_sample_t) * AUDIO_BUFFER_SIZE);
    stream.read(reinterpret_cast<char*> (&apu_state), sizeof(apu_state));

    Gb_Apu::mode_t mode = m_bCGB ? Gb_Apu::mode_cgb : Gb_Apu::mode_dmg;
    m_pApu->reset(mode);
    m_pApu->load_state(apu_state);
    m_pBuffer->clear();
}

Gb_Apu* Audio::GetApu()
{
    return m_pApu;
}

bool Audio::StartVgmRecording(const char* file_path, int clock_rate, bool is_double_speed)
{
    if (m_bVgmRecordingEnabled)
        return false;

    m_VgmRecorder.Start(file_path, clock_rate, is_double_speed);
    m_bVgmRecordingEnabled = m_VgmRecorder.IsRecording();

    // Write initial state of all audio registers to VGM
    if (m_bVgmRecordingEnabled)
    {
        // Get APU state without impacting emulation
        gb_apu_state_t apu_state;
        m_pApu->save_state(&apu_state);

        // First, ensure sound is enabled (NR52)
        m_VgmRecorder.WriteGbDmg(0xFF26, 0x80);

        // Write all audio control registers (FF10-FF26)
        for (u16 addr = 0xFF10; addr <= 0xFF26; addr++)
        {
            u8 value = apu_state.regs[addr - 0xFF10];
            m_VgmRecorder.WriteGbDmg(addr, value);
        }
        // Write wave RAM (FF30-FF3F)
        for (u16 addr = 0xFF30; addr <= 0xFF3F; addr++)
        {
            u8 value = apu_state.regs[addr - 0xFF10];
            m_VgmRecorder.WriteGbDmg(addr, value);
        }
    }

    return m_bVgmRecordingEnabled;
}

void Audio::StopVgmRecording()
{
    if (m_bVgmRecordingEnabled)
    {
        m_VgmRecorder.Stop();
        m_bVgmRecordingEnabled = false;
    }
}

bool Audio::IsVgmRecording() const
{
    return m_bVgmRecordingEnabled;
}
