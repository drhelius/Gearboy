/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#import "GearboyEmulator.h"

#import <AVFAudio/AVFAudio.h>

#include <string.h>
#include <strings.h>

#include "IOSAudioQueue.h"

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "miniz.h"
#undef MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#undef MIN
#undef MAX
#include "../../../src/gearboy.h"

bool g_mcp_stdio_mode = false;

static NSString* const GearboyEmulatorErrorDomain = @"me.ignaciosanchez.gearboy.emulator";

static bool IsROMArchiveEntry(const char* filename)
{
    const char* extension = strrchr(filename, '.');
    if (!extension)
        return false;

    return (strcasecmp(extension, ".gb") == 0) ||
           (strcasecmp(extension, ".dmg") == 0) ||
           (strcasecmp(extension, ".gbc") == 0) ||
           (strcasecmp(extension, ".cgb") == 0) ||
           (strcasecmp(extension, ".sgb") == 0) ||
           (strcasecmp(extension, ".bin") == 0) ||
           (strcasecmp(extension, ".rom") == 0);
}

static NSString* ROMCRCInArchive(NSURL* url)
{
    if (!url.isFileURL)
        return nil;

    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    if (!mz_zip_reader_init_file(&archive, url.fileSystemRepresentation, 0))
        return nil;

    NSString* result = nil;
    mz_uint fileCount = mz_zip_reader_get_num_files(&archive);
    for (mz_uint index = 0; index < fileCount; index++)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&archive, index, &fileStat))
            break;
        if (!IsROMArchiveEntry(fileStat.m_filename))
            continue;

        size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&archive, index, &size, 0);
        if (!data)
            break;

        mz_ulong checksum = mz_crc32(MZ_CRC32_INIT, (const unsigned char*)data, size);
        free(data);
        result = [NSString stringWithFormat:@"%08X", (unsigned int)checksum];
        break;
    }

    mz_zip_reader_end(&archive);
    return result;
}

static GB_Color kOriginalPalette[4] =
{
    {0x87, 0x96, 0x03}, {0x4D, 0x6B, 0x03}, {0x2B, 0x55, 0x03}, {0x14, 0x44, 0x03}
};
static GB_Color kSharpPalette[4] =
{
    {0xF5, 0xFA, 0xEF}, {0x86, 0xC2, 0x70}, {0x2F, 0x69, 0x57}, {0x0B, 0x19, 0x20}
};
static GB_Color kBlackAndWhitePalette[4] =
{
    {0xFF, 0xFF, 0xFF}, {0xAA, 0xAA, 0xAA}, {0x55, 0x55, 0x55}, {0x00, 0x00, 0x00}
};
static GB_Color kAutumnPalette[4] =
{
    {0xFF, 0xF6, 0xD3}, {0xF9, 0xA8, 0x75}, {0xEB, 0x6B, 0x6F}, {0x7C, 0x3F, 0x58}
};
static GB_Color kSoftPalette[4] =
{
    {0xE0, 0xE0, 0xAA}, {0xB0, 0xB8, 0x7C}, {0x72, 0x82, 0x5B}, {0x39, 0x34, 0x17}
};
static GB_Color kSlimePalette[4] =
{
    {0xD4, 0xEB, 0xA5}, {0x62, 0xB8, 0x7C}, {0x27, 0x76, 0x5D}, {0x1D, 0x39, 0x39}
};

static Cartridge::CartridgeTypes MapperForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Cartridge::CartridgeNoMBC;
        case 2:
            return Cartridge::CartridgeMBC1;
        case 3:
            return Cartridge::CartridgeMBC2;
        case 4:
            return Cartridge::CartridgeMBC3;
        case 5:
            return Cartridge::CartridgeMBC5;
        case 6:
            return Cartridge::CartridgeMBC1Multi;
        case 7:
            return Cartridge::CartridgeHuC1;
        case 8:
            return Cartridge::CartridgeHuC3;
        case 9:
            return Cartridge::CartridgeMMM01;
        case 10:
            return Cartridge::CartridgeCamera;
        case 11:
            return Cartridge::CartridgeMBC7;
        case 12:
            return Cartridge::CartridgeTAMA5;
        case 13:
            return Cartridge::CartridgeWisdomTree;
        case 14:
            return Cartridge::CartridgeM161;
        case 15:
            return Cartridge::CartridgeSachenMMC1;
        case 16:
            return Cartridge::CartridgeSachenMMC2;
        case 17:
            return Cartridge::CartridgePKJD;
        case 18:
            return Cartridge::CartridgeBungEMS;
        case 19:
            return Cartridge::CartridgePoke2in1;
        case 20:
            return Cartridge::CartridgeMBC6;
        default:
            return Cartridge::CartridgeNotSupported;
    }
}

@interface GearboyEmulator ()
{
    GearboyCore* m_core;
    u16* m_frameBuffer;
    s16* m_audioBuffer;
    IOSAudioQueue m_audioQueue;
    uint32_t m_pressedButtons;
    BOOL m_loaded;
    BOOL m_muted;
    BOOL m_forceDMG;
    BOOL m_forceGBA;
    BOOL m_superGameBoy;
    BOOL m_superGameBoyBorder;
    BOOL m_colorCorrection;
    BOOL m_noSpriteLimit;
    NSInteger m_palette;
    NSInteger m_saveStateSlot;
    NSInteger m_frameWidth;
    NSInteger m_frameHeight;
    double m_framesPerSecond;
    Cartridge::CartridgeTypes m_mapper;
    AVAudioEngine* m_audioEngine;
    AVAudioSourceNode* m_audioSourceNode;
}

- (void)applyConfiguration;
- (void)updateRuntimeInfo;
- (void)configureAudio;
- (void)audioEngineConfigurationChanged:(NSNotification*)notification;
- (void)clearAudio;
- (void)enqueueAudioSamples:(const s16*)samples count:(int)count;
- (OSStatus)renderAudioFrames:(AVAudioFrameCount)frameCount outputData:(AudioBufferList*)outputData silence:(BOOL*)isSilence;

@end

@implementation GearboyEmulator

+ (NSString*)romCRCInArchiveAtURL:(NSURL*)url
{
    return ROMCRCInArchive(url);
}

- (instancetype)init
{
    self = [super init];

    if (self)
    {
        m_core = new GearboyCore();
        m_core->Init(GB_PIXEL_RGB565);
        m_core->SetSoundSampleRate(GB_AUDIO_SAMPLE_RATE);

        m_frameBuffer = new u16[SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT]();
        m_audioBuffer = new s16[AUDIO_BUFFER_SIZE]();
        m_audioQueue.Configure(GB_AUDIO_QUEUE_SIZE, 3);
        m_pressedButtons = 0;
        m_loaded = NO;
        m_muted = NO;
        m_forceDMG = NO;
        m_forceGBA = NO;
        m_superGameBoy = YES;
        m_superGameBoyBorder = NO;
        m_colorCorrection = NO;
        m_noSpriteLimit = NO;
        m_palette = 0;
        m_saveStateSlot = 1;
        m_frameWidth = GAMEBOY_WIDTH;
        m_frameHeight = GAMEBOY_HEIGHT;
        m_framesPerSecond = 60.0;
        m_mapper = Cartridge::CartridgeNotSupported;

        [self applyConfiguration];
        [self configureAudio];
    }

    return self;
}

- (void)dealloc
{
    [NSNotificationCenter.defaultCenter removeObserver:self];
    [self stopAudio];

    if (m_loaded)
    {
        m_core->SaveRam();
    }

    SafeDeleteArray(m_audioBuffer);
    SafeDeleteArray(m_frameBuffer);
    SafeDelete(m_core);
}

- (void)configureWithModel:(NSInteger)model
                    mapper:(NSInteger)mapper
                   palette:(NSInteger)palette
           colorCorrection:(BOOL)colorCorrection
              noSpriteLimit:(BOOL)noSpriteLimit
               superGameBoy:(BOOL)superGameBoy
         superGameBoyBorder:(BOOL)superGameBoyBorder
              saveStateSlot:(NSInteger)saveStateSlot
{
    m_forceDMG = model == 1;
    m_forceGBA = model == 2;
    m_mapper = MapperForOption(mapper);
    m_palette = (palette >= 0 && palette <= 5) ? palette : 0;
    m_colorCorrection = colorCorrection;
    m_noSpriteLimit = noSpriteLimit;
    m_superGameBoy = superGameBoy;
    m_superGameBoyBorder = superGameBoyBorder;

    if (saveStateSlot < 1)
    {
        m_saveStateSlot = 1;
    }
    else if (saveStateSlot > 5)
    {
        m_saveStateSlot = 5;
    }
    else
    {
        m_saveStateSlot = saveStateSlot;
    }

    [self applyConfiguration];
}

- (void)applyConfiguration
{
    GB_Color* palette = kOriginalPalette;

    switch (m_palette)
    {
        case 1:
            palette = kSharpPalette;
            break;
        case 2:
            palette = kBlackAndWhitePalette;
            break;
        case 3:
            palette = kAutumnPalette;
            break;
        case 4:
            palette = kSoftPalette;
            break;
        case 5:
            palette = kSlimePalette;
            break;
        default:
            break;
    }

    m_core->SetDMGPalette(palette[0], palette[1], palette[2], palette[3]);
    m_core->EnableColorCorrection(m_colorCorrection);
    m_core->GetVideo()->SetNoSpriteLimit(m_noSpriteLimit);
    m_core->SetSGBEnabled(m_superGameBoy);
    m_core->SetSGBBorder(m_superGameBoyBorder);
}

- (void)updateRuntimeInfo
{
    GB_RuntimeInfo runtimeInfo;

    if (m_core->GetRuntimeInfo(runtimeInfo))
    {
        m_frameWidth = runtimeInfo.screen_width;
        m_frameHeight = runtimeInfo.screen_height;
        m_framesPerSecond = runtimeInfo.fps;
    }
}

- (BOOL)loadROMAtURL:(NSURL*)url error:(NSError**)error
{
    if (!url.isFileURL)
    {
        if (error)
        {
            *error = [NSError errorWithDomain:GearboyEmulatorErrorDomain
                                         code:1
                                     userInfo:@{NSLocalizedDescriptionKey: @"The selected item is not a local ROM file."}];
        }

        return NO;
    }

    if (m_loaded)
    {
        m_core->SaveRam();
    }

    [self releaseAllButtons];
    [self clearAudio];

    [self applyConfiguration];
    BOOL loaded = m_core->LoadROM(url.fileSystemRepresentation, m_forceDMG, m_mapper, m_forceGBA);

    if (!loaded)
    {
        m_loaded = NO;

        if (error)
        {
            *error = [NSError errorWithDomain:GearboyEmulatorErrorDomain
                                         code:2
                                     userInfo:@{NSLocalizedDescriptionKey: @"Gearboy could not load this ROM."}];
        }

        return NO;
    }

    m_core->LoadRam();
    m_core->Pause(false);
    m_loaded = YES;
    memset(m_frameBuffer, 0, SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT * sizeof(u16));
    [self updateRuntimeInfo];

    return YES;
}

- (void)runFrame
{
    if (!m_loaded || m_core->IsPaused())
    {
        return;
    }

    int sampleCount = 0;
    m_core->RunToVBlank(m_frameBuffer, m_audioBuffer, &sampleCount);
    [self updateRuntimeInfo];

    if (!m_muted && (sampleCount > 0))
    {
        [self enqueueAudioSamples:m_audioBuffer count:sampleCount];
    }
}

- (void)setButton:(GearboyButton)button pressed:(BOOL)pressed
{
    if (!m_loaded)
    {
        return;
    }

    uint32_t buttonMask = 1U << (uint32_t)button;
    BOOL wasPressed = (m_pressedButtons & buttonMask) != 0;

    if (pressed == wasPressed)
    {
        return;
    }

    Gameboy_Keys key;

    switch (button)
    {
        case GearboyButtonUp:
            key = Up_Key;
            break;
        case GearboyButtonDown:
            key = Down_Key;
            break;
        case GearboyButtonLeft:
            key = Left_Key;
            break;
        case GearboyButtonRight:
            key = Right_Key;
            break;
        case GearboyButtonA:
            key = A_Key;
            break;
        case GearboyButtonB:
            key = B_Key;
            break;
        case GearboyButtonStart:
            key = Start_Key;
            break;
        case GearboyButtonSelect:
            key = Select_Key;
            break;
    }

    if (pressed)
    {
        m_pressedButtons |= buttonMask;
        m_core->KeyPressed(key);
    }
    else
    {
        m_pressedButtons &= ~buttonMask;
        m_core->KeyReleased(key);
    }
}

- (void)releaseAllButtons
{
    if (!m_core)
    {
        return;
    }

    static const GearboyButton buttons[] =
    {
        GearboyButtonUp,
        GearboyButtonDown,
        GearboyButtonLeft,
        GearboyButtonRight,
        GearboyButtonA,
        GearboyButtonB,
        GearboyButtonStart,
        GearboyButtonSelect
    };

    for (GearboyButton button : buttons)
    {
        if ((m_pressedButtons & (1U << (uint32_t)button)) != 0)
        {
            [self setButton:button pressed:NO];
        }
    }
}

- (void)pause
{
    if (m_loaded)
    {
        [self releaseAllButtons];
        m_core->Pause(true);
    }
}

- (void)resume
{
    if (m_loaded)
    {
        m_core->Pause(false);
    }
}

- (void)reset
{
    if (!m_loaded)
    {
        return;
    }

    [self releaseAllButtons];
    m_core->SaveRam();
    [self applyConfiguration];
    m_core->ResetROM(m_forceDMG, m_mapper, m_forceGBA);
    m_core->LoadRam();
    [self updateRuntimeInfo];
    [self clearAudio];
}

- (void)saveRAM
{
    if (m_loaded)
    {
        m_core->SaveRam();
    }
}

- (void)saveState
{
    if (m_loaded)
    {
        m_core->SaveState((int)m_saveStateSlot);
    }
}

- (void)loadState
{
    if (m_loaded)
    {
        [self releaseAllButtons];
        m_core->LoadState((int)m_saveStateSlot);
        [self clearAudio];
    }
}

- (void)setAccelerometerX:(double)x y:(double)y
{
    m_core->SetAccelerometer(x, y);
}

- (BOOL)isLoaded
{
    return m_loaded;
}

- (BOOL)isPaused
{
    return !m_loaded || m_core->IsPaused();
}

- (BOOL)isColorGame
{
    return m_loaded && m_core->IsCGB();
}

- (BOOL)isTiltGame
{
    return m_loaded && ((m_mapper == Cartridge::CartridgeMBC7) ||
        (m_core->GetCartridge()->GetType() == Cartridge::CartridgeMBC7));
}

- (BOOL)isMuted
{
    return m_muted;
}

- (void)setMuted:(BOOL)muted
{
    m_muted = muted;

    if (muted)
    {
        [self clearAudio];
    }
}

- (const uint16_t*)frameBuffer
{
    return m_frameBuffer;
}

- (NSInteger)frameWidth
{
    return m_frameWidth;
}

- (NSInteger)frameHeight
{
    return m_frameHeight;
}

- (double)framesPerSecond
{
    return m_framesPerSecond;
}

- (void)configureAudio
{
    m_audioEngine = [[AVAudioEngine alloc] init];
    AVAudioFormat* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:GB_AUDIO_SAMPLE_RATE channels:2];
    __weak GearboyEmulator* weakSelf = self;

    m_audioSourceNode = [[AVAudioSourceNode alloc] initWithFormat:format
                                                     renderBlock:^OSStatus(BOOL* isSilence,
                                                                         const AudioTimeStamp* timestamp,
                                                                         AVAudioFrameCount frameCount,
                                                                         AudioBufferList* outputData)
    {
        UNUSED(timestamp);
        GearboyEmulator* strongSelf = weakSelf;

        if (!strongSelf)
        {
            *isSilence = YES;

            for (UInt32 bufferIndex = 0; bufferIndex < outputData->mNumberBuffers; ++bufferIndex)
            {
                AudioBuffer* buffer = &outputData->mBuffers[bufferIndex];
                memset(buffer->mData, 0, buffer->mDataByteSize);
            }

            return noErr;
        }

        return [strongSelf renderAudioFrames:frameCount outputData:outputData silence:isSilence];
    }];

    [m_audioEngine attachNode:m_audioSourceNode];
    [m_audioEngine connect:m_audioSourceNode to:m_audioEngine.mainMixerNode format:format];
    [m_audioEngine prepare];

    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(audioEngineConfigurationChanged:)
                                               name:AVAudioEngineConfigurationChangeNotification
                                             object:m_audioEngine];
}

- (void)startAudio
{
    if (m_audioEngine.isRunning)
    {
        return;
    }

    AVAudioSession* session = AVAudioSession.sharedInstance;
    NSError* error = nil;
    [session setCategory:AVAudioSessionCategoryAmbient
                    mode:AVAudioSessionModeDefault
                 options:AVAudioSessionCategoryOptionMixWithOthers
                   error:&error];

    if (!error)
    {
        NSError* preferenceError = nil;
        [session setPreferredSampleRate:GB_AUDIO_SAMPLE_RATE error:&preferenceError];
        preferenceError = nil;
        [session setPreferredIOBufferDuration:512.0 / GB_AUDIO_SAMPLE_RATE error:&preferenceError];
        [session setActive:YES error:&error];
    }

    [self clearAudio];

    if (!error)
    {
        [m_audioEngine startAndReturnError:&error];
    }

    if (error)
    {
        NSLog(@"Unable to start Gearboy audio: %@", error.localizedDescription);
    }
}

- (void)stopAudio
{
    if (m_audioEngine.isRunning)
    {
        [m_audioEngine pause];
    }

    [self clearAudio];

    NSError* error = nil;
    [AVAudioSession.sharedInstance setActive:NO
                                 withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
                                       error:&error];

    if (error)
    {
        NSLog(@"Unable to stop Gearboy audio: %@", error.localizedDescription);
    }
}

- (void)audioEngineConfigurationChanged:(NSNotification*)notification
{
    UNUSED(notification);
    [self clearAudio];

    __weak GearboyEmulator* weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        GearboyEmulator* strongSelf = weakSelf;

        if (!strongSelf || !strongSelf->m_loaded ||
            strongSelf->m_core->IsPaused() || strongSelf->m_audioEngine.isRunning)
        {
            return;
        }

        [strongSelf->m_audioEngine prepare];

        NSError* error = nil;
        [strongSelf->m_audioEngine startAndReturnError:&error];

        if (error)
        {
            NSLog(@"Unable to restart Gearboy audio: %@", error.localizedDescription);
        }
    });
}

- (void)clearAudio
{
    m_audioQueue.Reset();
}

- (void)enqueueAudioSamples:(const s16*)samples count:(int)count
{
    if (count > 0)
        m_audioQueue.Write(samples, (uint32_t)count);
}

- (OSStatus)renderAudioFrames:(AVAudioFrameCount)frameCount outputData:(AudioBufferList*)outputData silence:(BOOL*)isSilence
{
    bool audible = false;

    if (outputData->mNumberBuffers >= 2)
    {
        float* left = (float*)outputData->mBuffers[0].mData;
        float* right = (float*)outputData->mBuffers[1].mData;
        audible = m_audioQueue.Render(left, right, (uint32_t)frameCount);
    }
    else if (outputData->mNumberBuffers == 1)
    {
        float* output = (float*)outputData->mBuffers[0].mData;
        audible = m_audioQueue.RenderInterleaved(output, (uint32_t)frameCount);
    }

    *isSilence = !audible;
    return noErr;
}

@end
