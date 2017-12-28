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

#ifndef AUDIO_H
#define	AUDIO_H

#include "definitions.h"
#include "audio/Multi_Buffer.h"
#include "audio/Gb_Apu.h"

class Audio
{
public:
    Audio();
    ~Audio();
    void Init();
    void Reset(bool bCGB);
    void SetSampleRate(int rate);
    u8 ReadAudioRegister(u16 address);
    void WriteAudioRegister(u16 address, u8 value);
    void Tick(unsigned int clockCycles);
    void EndFrame(s16* pSampleBuffer, int* pSampleCount);

private:
    Gb_Apu* m_pApu;
    Stereo_Buffer* m_pBuffer;
    int m_ElapsedCycles;
    int m_SampleRate;
    blip_sample_t* m_pSampleBuffer;
    bool m_bCGB;
};

inline void Audio::Tick(unsigned int clockCycles)
{
    m_ElapsedCycles += clockCycles;
}

inline u8 Audio::ReadAudioRegister(u16 address)
{
    return m_pApu->read_register(m_ElapsedCycles, address);
}

inline void Audio::WriteAudioRegister(u16 address, u8 value)
{
    m_pApu->write_register(m_ElapsedCycles, address, value);
}

#endif	/* AUDIO_H */
