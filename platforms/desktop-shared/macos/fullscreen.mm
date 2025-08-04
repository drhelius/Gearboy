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

#import <Cocoa/Cocoa.h>
#include <functional>

// Simple bridge for C++ callback
@interface FullscreenObserver : NSObject
@property (nonatomic, copy) void (^onEnterFullscreen)(void);
@property (nonatomic, copy) void (^onExitFullscreen)(void);
@end

@implementation FullscreenObserver

- (instancetype)initWithWindow:(NSWindow *)window
{
    self = [super init];
    if (self)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowWillEnterFullScreen:)
                                                     name:NSWindowWillEnterFullScreenNotification
                                                   object:window];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowWillExitFullScreen:)
                                                     name:NSWindowWillExitFullScreenNotification
                                                   object:window];
    }
    return self;
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    if (self.onEnterFullscreen) self.onEnterFullscreen();
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    if (self.onExitFullscreen) self.onExitFullscreen();
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

@end

// C bridge
extern "C" void* macos_install_fullscreen_observer(void* nswindow,
                                             void(*enter_cb)(),
                                             void(*exit_cb)())
{
    FullscreenObserver* obs = [[FullscreenObserver alloc] initWithWindow:(__bridge NSWindow*)nswindow];
    if (enter_cb) obs.onEnterFullscreen = ^{ enter_cb(); };
    if (exit_cb) obs.onExitFullscreen = ^{ exit_cb(); };
    return (void*)obs;
}

extern "C" void macos_set_native_fullscreen(void* nswindow, bool enter)
{
    NSWindow* win = (__bridge NSWindow*)nswindow;
    BOOL isFullScreen = ([win styleMask] & NSWindowStyleMaskFullScreen) != 0;
    if (enter && !isFullScreen)
    {
        [win toggleFullScreen:nil];
    }
    else if (!enter && isFullScreen)
    {
        [win toggleFullScreen:nil];
    }
}
