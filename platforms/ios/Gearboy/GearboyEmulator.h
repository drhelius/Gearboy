/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, GearboyButton)
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

@interface GearboyEmulator : NSObject

@property (nonatomic, readonly, getter=isLoaded) BOOL loaded;
@property (nonatomic, readonly, getter=isPaused) BOOL paused;
@property (nonatomic, readonly, getter=isColorGame) BOOL colorGame;
@property (nonatomic, readonly, getter=isTiltGame) BOOL tiltGame;
@property (nonatomic, getter=isMuted) BOOL muted;
@property (nonatomic, readonly) const uint16_t* frameBuffer;
@property (nonatomic, readonly) NSInteger frameWidth;
@property (nonatomic, readonly) NSInteger frameHeight;
@property (nonatomic, readonly) double framesPerSecond;

+ (nullable NSString*)romCRCInArchiveAtURL:(NSURL*)url NS_SWIFT_NAME(romCRC(inArchiveAt:));
- (void)configureWithModel:(NSInteger)model
                    mapper:(NSInteger)mapper
                   palette:(NSInteger)palette
           colorCorrection:(BOOL)colorCorrection
              noSpriteLimit:(BOOL)noSpriteLimit
               superGameBoy:(BOOL)superGameBoy
         superGameBoyBorder:(BOOL)superGameBoyBorder
              saveStateSlot:(NSInteger)saveStateSlot
    NS_SWIFT_NAME(configure(model:mapper:palette:colorCorrection:noSpriteLimit:superGameBoy:superGameBoyBorder:saveStateSlot:));
- (BOOL)loadROMAtURL:(NSURL*)url error:(NSError* _Nullable* _Nullable)error NS_SWIFT_NAME(loadROM(at:));
- (void)runFrame;
- (void)setButton:(GearboyButton)button pressed:(BOOL)pressed;
- (void)setAccelerometerX:(double)x y:(double)y NS_SWIFT_NAME(setAccelerometer(x:y:));
- (void)releaseAllButtons;
- (void)pause;
- (void)resume;
- (void)reset;
- (void)saveRAM;
- (void)saveState;
- (void)loadState;
- (void)startAudio;
- (void)stopAudio;

@end

NS_ASSUME_NONNULL_END
