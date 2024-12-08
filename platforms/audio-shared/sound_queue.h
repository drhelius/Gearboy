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

#ifndef SOUND_QUEUE_H
#define SOUND_QUEUE_H

#include <SDL.h>
#include <stdint.h>
#include <assert.h>

class SoundQueue
{
public:
    SoundQueue();
    ~SoundQueue();
    bool Start(int sample_rate, int channel_count, int buffer_size = 2048, int buffer_count = 3);
    void Stop();
    void Write(int16_t* samples, int count, bool sync);
    int GetSampleCount();
    int16_t* GetCurrentlyPlaying();
    bool IsOpen();

private:
    int16_t* volatile m_buffers;
    SDL_sem* volatile m_free_sem;
    int16_t* volatile m_currently_playing;
    int volatile m_read_buffer;
    int m_write_buffer;
    int m_write_position;
    bool m_sound_open;
    bool m_sync_output;
    int m_buffer_size;
    int m_buffer_count;

private:
    int16_t* Buffer(int index);
    void FillBuffer(uint8_t* buffer, int count);
    bool IsRunningInWSL();
    static void FillBufferCallback(void* user_data, uint8_t* buffer, int count);
};

inline int16_t* SoundQueue::Buffer(int index)
{
    assert(index < m_buffer_count);
    return m_buffers + (index * m_buffer_size);
}

#endif /* SOUND_QUEUE_H */