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

#include <string>
#define SOUND_QUEUE_IMPORT
#include "sound_queue.h"
#include "utils.h"

#define SOUND_QUEUE_DEBUG(...) { }
//#define SOUND_QUEUE_DEBUG(x, ...) Debug(x, ## __VA_ARGS__)

static SDL_AudioStream* sound_queue_stream;
static bool sound_queue_sound_open;
static bool sound_queue_playing;
static int sound_queue_max_queued_bytes;
static int sound_queue_bytes_per_second;
static float sound_queue_ratio;
static int sound_queue_sync_wait_count;

static const float k_sound_queue_max_ratio_delta = 0.005f;

static void sound_queue_update_ratio(int queued, int bytes, bool rate_control, float base_ratio);
static bool is_running_in_wsl(void);

void sound_queue_init(void)
{
    InitPointer(sound_queue_stream);
    sound_queue_sound_open = false;
    sound_queue_playing = false;
    sound_queue_sync_wait_count = 0;

    SDL_SetHintWithPriority(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "512", SDL_HINT_DEFAULT);

    int audio_drivers_count = SDL_GetNumAudioDrivers();

    Debug("Sound Queue: %d audio drivers", audio_drivers_count);

    for (int i = 0; i < audio_drivers_count; i++)
    {
        Log("Sound Queue: Driver [%s]", SDL_GetAudioDriver(i));
    }

    std::string platform = SDL_GetPlatform();
    if (platform == "Linux")
    {
        if (is_running_in_wsl())
        {
            Debug("Sound Queue: Running in WSL");
            SDL_SetHint("SDL_AUDIODRIVER", "pulseaudio");
        }
        else
        {
            Debug("Sound Queue: Running in Linux");
        }
    }

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
        SDL_ERROR("SDL_InitSubSystem(SDL_INIT_AUDIO)");

    Log("Sound Queue: [%s] driver selected", SDL_GetCurrentAudioDriver());

    int audio_devices_count = 0;
    SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&audio_devices_count);

    Debug("Sound Queue: %d audio devices", audio_devices_count);

    if (devices)
    {
        for (int i = 0; i < audio_devices_count; i++)
        {
            Log("Sound Queue: Device [%s]", SDL_GetAudioDeviceName(devices[i]));
        }
        SDL_free(devices);
    }
}

void sound_queue_destroy(void)
{
    sound_queue_stop();
}

bool sound_queue_start(int sample_rate, int channel_count, int buffer_size, int buffer_count)
{
    Debug("Sound Queue: Starting with %d Hz, %d channels, %d buffer size, %d buffers ...", sample_rate, channel_count, buffer_size, buffer_count);

    sound_queue_max_queued_bytes = buffer_size * buffer_count * (int)sizeof(s16);
    sound_queue_bytes_per_second = sample_rate * channel_count * (int)sizeof(s16);

    SDL_AudioSpec spec;
    spec.freq = sample_rate;
    spec.format = SDL_AUDIO_S16;
    spec.channels = channel_count;

    Debug("Sound Queue: Spec - frequency: %d format: 0x%04X channels: %d", spec.freq, spec.format, spec.channels);

    sound_queue_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

    if (!sound_queue_stream)
    {
        SDL_ERROR("SDL_OpenAudioDeviceStream");
        return false;
    }

    SDL_AudioDeviceID selected_device = SDL_GetAudioStreamDevice(sound_queue_stream);

    Log("Sound Queue: Started [%s] - frequency: %d format: 0x%04X channels: %d", SDL_GetAudioDeviceName(selected_device), spec.freq, spec.format, spec.channels);

    sound_queue_playing = false;
    sound_queue_sound_open = true;
    sound_queue_ratio = 1.0f;

    return true;
}

void sound_queue_stop(void)
{
    if (sound_queue_sound_open)
    {
        sound_queue_sound_open = false;
        sound_queue_playing = false;
        if (sound_queue_stream)
        {
            SDL_PauseAudioStreamDevice(sound_queue_stream);
            SDL_DestroyAudioStream(sound_queue_stream);
            InitPointer(sound_queue_stream);
        }

        Debug("Sound Queue: Stopped");
    }
}

void sound_queue_flush(void)
{
    if (!sound_queue_sound_open || !sound_queue_stream)
        return;

    SDL_PauseAudioStreamDevice(sound_queue_stream);
    SDL_ClearAudioStream(sound_queue_stream);
    sound_queue_playing = false;

    if ((sound_queue_ratio != 1.0f) && SDL_SetAudioStreamFrequencyRatio(sound_queue_stream, 1.0f))
        sound_queue_ratio = 1.0f;
}

float sound_queue_get_target_latency_ms(void)
{
    if (sound_queue_bytes_per_second <= 0)
        return 0.0f;

    return (sound_queue_max_queued_bytes * 1000.0f) / sound_queue_bytes_per_second;
}

bool sound_queue_is_open(void)
{
    return sound_queue_sound_open;
}

int sound_queue_get_sync_wait_count(void)
{
    return sound_queue_sync_wait_count;
}

void sound_queue_write(s16* samples, int count, bool sync, bool rate_control, float base_ratio)
{
    if (!sound_queue_sound_open || !sound_queue_stream)
        return;

    int bytes = count * (int)sizeof(s16);
    int queued = SDL_GetAudioStreamQueued(sound_queue_stream);
    int max_queued = rate_control ? sound_queue_max_queued_bytes + bytes : sound_queue_max_queued_bytes;

    if (bytes > sound_queue_max_queued_bytes)
    {
        Log("Sound Queue: Write exceeds queue size (%d > %d bytes)", bytes, sound_queue_max_queued_bytes);
    }

    if (queued == 0)
    {
        SOUND_QUEUE_DEBUG("Sound Queue: Underrun detected, queue was empty");
    }

    if (sound_queue_playing)
        sound_queue_update_ratio(queued, bytes, rate_control, base_ratio);

    if (sound_queue_playing && sync)
    {
        int room = max_queued - queued;
        if (room < bytes)
        {
            SOUND_QUEUE_DEBUG("Sound Queue: Sync wait, need %d bytes but only %d free (queued %d, max %d)", bytes, room, queued, max_queued);
            int needed = bytes - room;
            int wait_ms = (needed * 1000) / sound_queue_bytes_per_second;
            if (wait_ms >= 1)
            {
                sound_queue_sync_wait_count++;
                SDL_Delay(wait_ms);
            }
        }
    }
    else if (sound_queue_playing)
    {
        if (queued >= max_queued)
        {
            SOUND_QUEUE_DEBUG("Sound Queue: Async overrun, dropping frame (queued %d >= max %d)", queued, max_queued);
            return;
        }
    }

    if (!SDL_PutAudioStreamData(sound_queue_stream, samples, bytes))
    {
        Log("Sound Queue: Unable to queue audio: %s", SDL_GetError());
        return;
    }

    if (!sound_queue_playing)
    {
        int queued_after = queued + bytes;

        if ((queued_after + bytes) > sound_queue_max_queued_bytes)
        {
            if (!SDL_ResumeAudioStreamDevice(sound_queue_stream))
            {
                SDL_ERROR("SDL_ResumeAudioStreamDevice");
                sound_queue_stop();
                return;
            }

            sound_queue_playing = true;
        }
    }
}

static void sound_queue_update_ratio(int queued, int bytes, bool rate_control, float base_ratio)
{
    float ratio = 1.0f;

    if (rate_control)
    {
        int target = sound_queue_max_queued_bytes - bytes;

        if (target < sound_queue_max_queued_bytes / 2)
            target = sound_queue_max_queued_bytes / 2;

        float error = (float)(target - queued) / (float)target;

        if (error < -1.0f)
            error = -1.0f;

        ratio = base_ratio * (1.0f - (error * k_sound_queue_max_ratio_delta));
    }

    if (ratio == sound_queue_ratio)
        return;

    if (SDL_SetAudioStreamFrequencyRatio(sound_queue_stream, ratio))
        sound_queue_ratio = ratio;
}

static bool is_running_in_wsl(void)
{
    FILE *file;

    if ((file = fopen("/proc/sys/fs/binfmt_misc/WSLInterop", "r")))
    {
        fclose(file);
        return true;
    }

    return false;
}
