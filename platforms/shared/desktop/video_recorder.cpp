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

#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include "stb_image_write.h"

#define VIDEO_RECORDER_IMPORT
#include "video_recorder.h"

#define VIDEO_RECORDER_RATE_SCALE 1000000
#define VIDEO_RECORDER_CHANNELS 2
#define VIDEO_RECORDER_BLOCK_ALIGN (VIDEO_RECORDER_CHANNELS * 2)
#define VIDEO_RECORDER_SEGMENT_SIZE 0x40000000
#define VIDEO_RECORDER_MAX_SEGMENTS 1024

#define AVIF_HASINDEX 0x00000010
#define AVIF_ISINTERLEAVED 0x00000100
#define AVIF_TRUSTCKTYPE 0x00000800
#define AVIIF_KEYFRAME 0x00000010
#define AVI_INDEX_OF_INDEXES 0x00
#define AVI_INDEX_OF_CHUNKS 0x01

enum Video_Recorder_Stream
{
    Video_Recorder_Stream_Video = 0,
    Video_Recorder_Stream_Audio,
    Video_Recorder_Stream_Count
};

struct Video_Recorder_Chunk
{
    u32 offset;
    u32 size;
};

struct Video_Recorder_Segment
{
    u64 offset;
    u32 size;
    u32 duration;
};

static const int k_jpeg_quality[Video_Recorder_Quality_Lossless] = { 75, 90, 95 };
static const char* const k_chunk_ids[Video_Recorder_Stream_Count] = { "00dc", "01wb" };
static const char* const k_index_ids[Video_Recorder_Stream_Count] = { "ix00", "ix01" };

static std::ofstream file;
static std::string file_path;
static bool recording = false;
static int width = 0;
static int height = 0;
static Video_Recorder_Quality quality = Video_Recorder_Quality_High;
static u32 rate = 0;
static u32 sample_rate = 0;
static u64 position = 0;
static u64 riff_start = 0;
static u64 movi_size_position = 0;
static u32 first_riff_size = 0;
static u32 first_riff_frames = 0;
static int segment_count = 0;
static u32 frame_count = 0;
static u64 video_bytes = 0;
static u64 audio_bytes = 0;
static u32 max_chunk_size[Video_Recorder_Stream_Count] = { 0 };
static std::vector<Video_Recorder_Chunk> chunks[Video_Recorder_Stream_Count];
static Video_Recorder_Segment segments[Video_Recorder_Stream_Count][VIDEO_RECORDER_MAX_SEGMENTS];
static std::vector<u8> frame_data;
static u32* scaled_buffer = NULL;
static int* scaled_x = NULL;
static int scaled_source_width = 0;

static void write_data(const void* data, u32 size);
static void write_u8(u8 value);
static void write_u16(u16 value);
static void write_u32(u32 value);
static void write_u64(u64 value);
static void write_fourcc(const char* fourcc);
static void write_zeros(u32 size);
static void patch_u32(u64 at, u32 value);
static u64 begin_list(const char* type, const char* name);
static void end_list(u64 size_position);
static void write_header(void);
static void write_super_index(int stream);
static void write_standard_index(int stream);
static void write_legacy_index(void);
static void write_chunk(int stream, const void* data, u32 size);
static void begin_segment(void);
static void end_segment(void);
static bool check_segment(void);
static void scale_frame(const u8* frame_buffer, int frame_width, int frame_height, int bytes_per_pixel);
static void encode_lossless(void);
static int get_lossless_stride(void);
static void jpeg_write(void* context, void* data, int size);

bool video_recorder_start(const char* path, int video_width, int video_height, double fps, int audio_sample_rate, Video_Recorder_Quality video_quality)
{
    if (recording)
        video_recorder_stop();

    if ((video_width <= 0) || (video_height <= 0) || (fps <= 0.0) || (audio_sample_rate <= 0))
        return false;

    open_ofstream_utf8(file, path, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        Log("Video recording: unable to open %s", path);
        return false;
    }

    file_path = path;
    width = video_width;
    height = video_height;
    quality = video_quality;
    rate = (u32)((fps * VIDEO_RECORDER_RATE_SCALE) + 0.5);
    sample_rate = (u32)audio_sample_rate;
    position = 0;
    riff_start = 0;
    first_riff_size = 0;
    first_riff_frames = 0;
    segment_count = 0;
    frame_count = 0;
    video_bytes = 0;
    audio_bytes = 0;
    max_chunk_size[Video_Recorder_Stream_Video] = 0;
    max_chunk_size[Video_Recorder_Stream_Audio] = 0;
    memset(segments, 0, sizeof(segments));

    scaled_buffer = new u32[width * height];
    scaled_x = new int[width];
    scaled_source_width = 0;

    write_header();
    begin_segment();

    recording = true;
    return true;
}

void video_recorder_stop(void)
{
    if (!recording)
        return;

    recording = false;

    end_segment();
    file.seekp(0);
    position = 0;
    write_header();
    file.close();

    for (int i = 0; i < Video_Recorder_Stream_Count; i++)
        std::vector<Video_Recorder_Chunk>().swap(chunks[i]);

    std::vector<u8>().swap(frame_data);
    SafeDeleteArray(scaled_buffer);
    SafeDeleteArray(scaled_x);
}

bool video_recorder_is_recording(void)
{
    return recording;
}

const char* video_recorder_get_file_path(void)
{
    return file_path.c_str();
}

u32 video_recorder_get_frame_count(void)
{
    return frame_count;
}

void video_recorder_add_audio(const s16* samples, int count)
{
    if (!recording || (count <= 0) || !check_segment())
        return;

    u32 size = (u32)count * sizeof(s16);
    write_chunk(Video_Recorder_Stream_Audio, samples, size);
    audio_bytes += size;
}

void video_recorder_add_video(const u8* frame_buffer, int frame_width, int frame_height, int bytes_per_pixel)
{
    if (!recording || (frame_width <= 0) || (frame_height <= 0) || !check_segment())
        return;

    scale_frame(frame_buffer, frame_width, frame_height, bytes_per_pixel);

    if (quality == Video_Recorder_Quality_Lossless)
        encode_lossless();
    else
    {
        frame_data.clear();
        if (!stbi_write_jpg_to_func(jpeg_write, NULL, width, height, 4, scaled_buffer, k_jpeg_quality[quality]))
            return;
    }

    u32 size = (u32)frame_data.size();
    write_chunk(Video_Recorder_Stream_Video, frame_data.data(), size);
    video_bytes += size;
    frame_count++;

    if (!file.good())
    {
        Error("Video recording: unable to write %s", file_path.c_str());
        video_recorder_stop();
    }
}

static void write_data(const void* data, u32 size)
{
    file.write((const char*)data, size);
    position += size;
}

static void write_u8(u8 value)
{
    write_data(&value, 1);
}

static void write_u16(u16 value)
{
    u8 data[2] = { (u8)(value & 0xFF), (u8)(value >> 8) };
    write_data(data, 2);
}

static void write_u32(u32 value)
{
    u8 data[4] = { (u8)(value & 0xFF), (u8)((value >> 8) & 0xFF), (u8)((value >> 16) & 0xFF), (u8)(value >> 24) };
    write_data(data, 4);
}

static void write_u64(u64 value)
{
    write_u32((u32)(value & 0xFFFFFFFF));
    write_u32((u32)(value >> 32));
}

static void write_fourcc(const char* fourcc)
{
    write_data(fourcc, 4);
}

static void write_zeros(u32 size)
{
    for (u32 i = 0; i < size; i++)
        write_u8(0);
}

static void patch_u32(u64 at, u32 value)
{
    u8 data[4] = { (u8)(value & 0xFF), (u8)((value >> 8) & 0xFF), (u8)((value >> 16) & 0xFF), (u8)(value >> 24) };
    file.seekp((std::streamoff)at);
    file.write((const char*)data, 4);
    file.seekp((std::streamoff)position);
}

static u64 begin_list(const char* type, const char* name)
{
    write_fourcc(type);
    u64 size_position = position;
    write_u32(0);
    write_fourcc(name);
    return size_position;
}

static void end_list(u64 size_position)
{
    patch_u32(size_position, (u32)(position - size_position - 4));
}

static void write_header(void)
{
    double fps = (double)rate / VIDEO_RECORDER_RATE_SCALE;
    u32 micro_sec_per_frame = (u32)((((u64)VIDEO_RECORDER_RATE_SCALE * 1000000) + (rate / 2)) / rate);
    u32 max_bytes_per_sec = (frame_count > 0) ? (u32)(((double)(video_bytes + audio_bytes) * fps) / frame_count) : 0;

    write_fourcc("RIFF");
    write_u32(first_riff_size);
    write_fourcc("AVI ");

    u64 hdrl = begin_list("LIST", "hdrl");

    write_fourcc("avih");
    write_u32(56);
    write_u32(micro_sec_per_frame);
    write_u32(max_bytes_per_sec);
    write_u32(0);
    write_u32(AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE);
    write_u32(first_riff_frames);
    write_u32(0);
    write_u32(Video_Recorder_Stream_Count);
    write_u32(max_chunk_size[Video_Recorder_Stream_Video] + max_chunk_size[Video_Recorder_Stream_Audio]);
    write_u32(width);
    write_u32(height);
    write_zeros(16);

    u64 strl = begin_list("LIST", "strl");

    bool lossless = (quality == Video_Recorder_Quality_Lossless);

    write_fourcc("strh");
    write_u32(56);
    write_fourcc("vids");
    if (lossless)
        write_u32(0);
    else
        write_fourcc("MJPG");
    write_u32(0);
    write_u16(0);
    write_u16(0);
    write_u32(0);
    write_u32(VIDEO_RECORDER_RATE_SCALE);
    write_u32(rate);
    write_u32(0);
    write_u32(frame_count);
    write_u32(max_chunk_size[Video_Recorder_Stream_Video]);
    write_u32(0xFFFFFFFF);
    write_u32(0);
    write_u16(0);
    write_u16(0);
    write_u16((u16)width);
    write_u16((u16)height);

    write_fourcc("strf");
    write_u32(40);
    write_u32(40);
    write_u32(width);
    write_u32(height);
    write_u16(1);
    write_u16(24);
    if (lossless)
        write_u32(0);
    else
        write_fourcc("MJPG");
    write_u32(get_lossless_stride() * height);
    write_zeros(16);

    write_super_index(Video_Recorder_Stream_Video);
    end_list(strl);

    strl = begin_list("LIST", "strl");

    write_fourcc("strh");
    write_u32(56);
    write_fourcc("auds");
    write_u32(0);
    write_u32(0);
    write_u16(0);
    write_u16(0);
    write_u32(0);
    write_u32(1);
    write_u32(sample_rate);
    write_u32(0);
    write_u32((u32)(audio_bytes / VIDEO_RECORDER_BLOCK_ALIGN));
    write_u32(max_chunk_size[Video_Recorder_Stream_Audio]);
    write_u32(0xFFFFFFFF);
    write_u32(VIDEO_RECORDER_BLOCK_ALIGN);
    write_zeros(8);

    write_fourcc("strf");
    write_u32(16);
    write_u16(1);
    write_u16(VIDEO_RECORDER_CHANNELS);
    write_u32(sample_rate);
    write_u32(sample_rate * VIDEO_RECORDER_BLOCK_ALIGN);
    write_u16(VIDEO_RECORDER_BLOCK_ALIGN);
    write_u16(16);

    write_super_index(Video_Recorder_Stream_Audio);
    end_list(strl);

    u64 odml = begin_list("LIST", "odml");

    write_fourcc("dmlh");
    write_u32(248);
    write_u32(frame_count);
    write_zeros(244);

    end_list(odml);
    end_list(hdrl);
}

static void write_super_index(int stream)
{
    write_fourcc("indx");
    write_u32(24 + (16 * VIDEO_RECORDER_MAX_SEGMENTS));
    write_u16(4);
    write_u8(0);
    write_u8(AVI_INDEX_OF_INDEXES);
    write_u32(segment_count);
    write_fourcc(k_chunk_ids[stream]);
    write_zeros(12);

    for (int i = 0; i < VIDEO_RECORDER_MAX_SEGMENTS; i++)
    {
        write_u64(segments[stream][i].offset);
        write_u32(segments[stream][i].size);
        write_u32(segments[stream][i].duration);
    }
}

static void write_standard_index(int stream)
{
    std::vector<Video_Recorder_Chunk>& list = chunks[stream];
    u32 count = (u32)list.size();
    u32 duration = count;

    if (stream == Video_Recorder_Stream_Audio)
    {
        u64 bytes = 0;
        for (u32 i = 0; i < count; i++)
            bytes += list[i].size;
        duration = (u32)(bytes / VIDEO_RECORDER_BLOCK_ALIGN);
    }

    Video_Recorder_Segment& segment = segments[stream][segment_count];
    segment.offset = position;
    segment.size = 32 + (count * 8);
    segment.duration = duration;

    write_fourcc(k_index_ids[stream]);
    write_u32(24 + (count * 8));
    write_u16(2);
    write_u8(0);
    write_u8(AVI_INDEX_OF_CHUNKS);
    write_u32(count);
    write_fourcc(k_chunk_ids[stream]);
    write_u64(movi_size_position + 4);
    write_u32(0);

    for (u32 i = 0; i < count; i++)
    {
        write_u32(list[i].offset);
        write_u32(list[i].size);
    }
}

static void write_legacy_index(void)
{
    std::vector<Video_Recorder_Chunk>& video = chunks[Video_Recorder_Stream_Video];
    std::vector<Video_Recorder_Chunk>& audio = chunks[Video_Recorder_Stream_Audio];
    size_t v = 0;
    size_t a = 0;

    write_fourcc("idx1");
    write_u32((u32)((video.size() + audio.size()) * 16));

    while ((v < video.size()) || (a < audio.size()))
    {
        bool is_video = (a >= audio.size()) || ((v < video.size()) && (video[v].offset < audio[a].offset));
        const Video_Recorder_Chunk& chunk = is_video ? video[v++] : audio[a++];

        write_fourcc(k_chunk_ids[is_video ? Video_Recorder_Stream_Video : Video_Recorder_Stream_Audio]);
        write_u32(AVIIF_KEYFRAME);
        write_u32(chunk.offset - 8);
        write_u32(chunk.size);
    }
}

static void write_chunk(int stream, const void* data, u32 size)
{
    Video_Recorder_Chunk chunk;
    chunk.offset = (u32)(position + 8 - (movi_size_position + 4));
    chunk.size = size;
    chunks[stream].push_back(chunk);

    if (size > max_chunk_size[stream])
        max_chunk_size[stream] = size;

    write_fourcc(k_chunk_ids[stream]);
    write_u32(size);
    write_data(data, size);

    if (size & 1)
        write_u8(0);
}

static void begin_segment(void)
{
    if (segment_count > 0)
    {
        riff_start = position;
        begin_list("RIFF", "AVIX");
    }

    movi_size_position = begin_list("LIST", "movi");

    for (int i = 0; i < Video_Recorder_Stream_Count; i++)
        chunks[i].clear();
}

static void end_segment(void)
{
    for (int i = 0; i < Video_Recorder_Stream_Count; i++)
        write_standard_index(i);

    end_list(movi_size_position);

    if (segment_count == 0)
    {
        write_legacy_index();
        first_riff_size = (u32)(position - 8);
        first_riff_frames = frame_count;
    }
    else
        end_list(riff_start + 4);

    segment_count++;
}

static bool check_segment(void)
{
    if ((position - riff_start) < VIDEO_RECORDER_SEGMENT_SIZE)
        return true;

    if (segment_count >= (VIDEO_RECORDER_MAX_SEGMENTS - 1))
    {
        Log("Video recording: maximum file size reached");
        video_recorder_stop();
        return false;
    }

    end_segment();
    begin_segment();
    return true;
}

static void scale_frame(const u8* frame_buffer, int frame_width, int frame_height, int bytes_per_pixel)
{
    if (frame_width != scaled_source_width)
    {
        for (int x = 0; x < width; x++)
            scaled_x[x] = (((2 * x) + 1) * frame_width) / (2 * width);
        scaled_source_width = frame_width;
    }

    int last_y = -1;

    for (int y = 0; y < height; y++)
    {
        u32* dst_line = scaled_buffer + (y * width);
        int src_y = (((2 * y) + 1) * frame_height) / (2 * height);

        if (src_y == last_y)
        {
            memcpy(dst_line, dst_line - width, width * sizeof(u32));
            continue;
        }

        const u8* src_line = frame_buffer + (src_y * frame_width * bytes_per_pixel);

        if (bytes_per_pixel == 4)
        {
            const u32* src_pixels = (const u32*)src_line;

            for (int x = 0; x < width; x++)
                dst_line[x] = src_pixels[scaled_x[x]];
        }
        else
        {
            u8* dst_bytes = (u8*)dst_line;

            for (int x = 0; x < width; x++)
            {
                const u8* src_pixel = src_line + (scaled_x[x] * 3);
                dst_bytes[(x * 4) + 0] = src_pixel[0];
                dst_bytes[(x * 4) + 1] = src_pixel[1];
                dst_bytes[(x * 4) + 2] = src_pixel[2];
                dst_bytes[(x * 4) + 3] = 0xFF;
            }
        }

        last_y = src_y;
    }
}

static void encode_lossless(void)
{
    int stride = get_lossless_stride();
    frame_data.assign(stride * height, 0);

    for (int y = 0; y < height; y++)
    {
        const u8* src_line = (const u8*)(scaled_buffer + ((height - 1 - y) * width));
        u8* dst_line = frame_data.data() + (y * stride);

        for (int x = 0; x < width; x++)
        {
            dst_line[(x * 3) + 0] = src_line[(x * 4) + 2];
            dst_line[(x * 3) + 1] = src_line[(x * 4) + 1];
            dst_line[(x * 3) + 2] = src_line[(x * 4) + 0];
        }
    }
}

static int get_lossless_stride(void)
{
    return (((width * 3) + 3) / 4) * 4;
}

static void jpeg_write(void* context, void* data, int size)
{
    UNUSED(context);
    const u8* bytes = (const u8*)data;
    frame_data.insert(frame_data.end(), bytes, bytes + size);
}
