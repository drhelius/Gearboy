/*
 * Geargrafx - PC Engine / TurboGrafx Emulator
 * Copyright (C) 2024  Ignacio Sanchez

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

#include "sound_queue.h"
#include <string>
#include <assert.h>
#include "../../src/gearboy.h"

static void sdl_error(const char* str)
{
    const char* sdl_str = SDL_GetError();
    if (sdl_str && *sdl_str)
        str = sdl_str;

    Log("Sound Queue: %s", str);
}

SoundQueue::SoundQueue()
{
    m_buffers = NULL;
    m_free_sem = NULL;
    m_sound_open = false;
    m_sync_output = true;

    int audio_drivers_count = SDL_GetNumAudioDrivers();
    int audio_devices_count = SDL_GetNumAudioDevices(0);

    Debug("SoundQueue: %d audio backends", audio_drivers_count);

    for (int i = 0; i < audio_drivers_count; i++)
    {
        Debug("SoundQueue: %s", SDL_GetAudioDriver(i));
    }

    Debug("SoundQueue: %d audio devices", audio_devices_count);

    for (int i = 0; i < audio_devices_count; i++)
    {
        Debug("SoundQueue: %s", SDL_GetAudioDeviceName(i, 0));
    }

    std::string platform = SDL_GetPlatform();
    if (platform == "Linux")
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            sdl_error("Couldn't init AUDIO subsystem");

        if (IsRunningInWSL())
        {
            Debug("SoundQueue: Running in WSL");
            if (SDL_AudioInit("pulseaudio") < 0)
                sdl_error("Couldn't init pulseaudio audio driver");
        }
        else
        {
            Debug("SoundQueue: Running in Linux");
            if (SDL_AudioInit("alsa") < 0)
                sdl_error("Couldn't init alsa audio driver");
        }
    }
    else
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
            sdl_error("Couldn't init AUDIO");
    }

    Log("SoundQueue: %s driver selected", SDL_GetCurrentAudioDriver());

    atexit(SDL_Quit);
}

SoundQueue::~SoundQueue()
{
    Stop();
}

bool SoundQueue::Start(int sample_rate, int channel_count, int buffer_size, int buffer_count)
{
    Log("SoundQueue: Starting with %d Hz, %d channels, %d buffer size, %d buffers ...", sample_rate, channel_count, buffer_size, buffer_count);

    m_write_buffer = 0;
    m_write_position = 0;
    m_read_buffer = 0;
    m_buffer_size = buffer_size;
    m_buffer_count = buffer_count;

    m_buffers = new int16_t[m_buffer_size * m_buffer_count];
    m_currently_playing = m_buffers;

    for (int i = 0; i < (m_buffer_size * m_buffer_count); i++)
        m_buffers[i] = 0;

    m_free_sem = SDL_CreateSemaphore(m_buffer_count - 1);
    if (!m_free_sem)
    {
        sdl_error("Couldn't create semaphore");
        return false;
    }

    SDL_AudioSpec spec;
    spec.freq = sample_rate;
    spec.format = AUDIO_S16SYS;
    spec.channels = channel_count;
    spec.silence = 0;
    spec.samples = m_buffer_size / channel_count;
    spec.size = 0;
    spec.callback = FillBufferCallback;
    spec.userdata = this;

    Log("SoundQueue: Desired - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d", spec.freq, SDL_AUDIO_ISFLOAT(spec.format), SDL_AUDIO_ISSIGNED(spec.format), SDL_AUDIO_ISBIGENDIAN(spec.format), SDL_AUDIO_BITSIZE(spec.format), spec.channels, spec.samples);

    if (SDL_OpenAudio(&spec, NULL) < 0)
    {
        sdl_error("Couldn't open SDL audio");
        return false;
    }

    Log("SoundQueue: Obtained - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d", spec.freq, SDL_AUDIO_ISFLOAT(spec.format), SDL_AUDIO_ISSIGNED(spec.format), SDL_AUDIO_ISBIGENDIAN(spec.format), SDL_AUDIO_BITSIZE(spec.format), spec.channels, spec.samples);

    SDL_PauseAudio(false);
    m_sound_open = true;

    return true;
}

void SoundQueue::Stop()
{
    if (m_sound_open)
    {
        m_sound_open = false;
        SDL_PauseAudio(true);
        SDL_CloseAudio();
    }

    if (m_free_sem)
    {
        SDL_DestroySemaphore(m_free_sem);
        m_free_sem = NULL;
    }

    delete [] m_buffers;
    m_buffers = NULL;
}

int SoundQueue::GetSampleCount()
{
    int buffer_free = SDL_SemValue(m_free_sem) * m_buffer_size + (m_buffer_size - m_write_position);
    return m_buffer_size * m_buffer_count - buffer_free;
}

int16_t* SoundQueue::GetCurrentlyPlaying()
{
    return m_currently_playing;
}

bool SoundQueue::IsOpen()
{
    return m_sound_open;
}

void SoundQueue::Write(int16_t* samples, int count, bool sync)
{
    if (!m_sound_open)
        return;

    m_sync_output = sync;

    while (count)
    {
        int n = m_buffer_size - m_write_position;
        if (n > count)
            n = count;

        memcpy(Buffer(m_write_buffer) + m_write_position, samples, n * sizeof(int16_t));
        samples += n;
        m_write_position += n;
        count -= n;

        if (m_write_position >= m_buffer_size)
        {
            m_write_position = 0;
            m_write_buffer = (m_write_buffer + 1) % m_buffer_count;
            
            if (m_sync_output)
                SDL_SemWait(m_free_sem);
        }
    }
}

void SoundQueue::FillBuffer(uint8_t* buffer, int count)
{
    if ((SDL_SemValue(m_free_sem) < (unsigned int)m_buffer_count - 1) || !m_sync_output)
    {
        m_currently_playing = Buffer(m_read_buffer);
        memcpy( buffer, Buffer(m_read_buffer), count);
        m_read_buffer = (m_read_buffer + 1) % m_buffer_count;

        if (m_sync_output)
            SDL_SemPost(m_free_sem);
    }
    else
    {
        memset(buffer, 0, count);
    }
}

void SoundQueue::FillBufferCallback(void* user_data, uint8_t* buffer, int count)
{
    ((SoundQueue*) user_data)->FillBuffer(buffer, count);
}

bool SoundQueue::IsRunningInWSL()
{
    FILE *file;

    if ((file = fopen("/proc/sys/fs/binfmt_misc/WSLInterop", "r")))
    {
        fclose(file);
        return true;
    }

    return false;
}