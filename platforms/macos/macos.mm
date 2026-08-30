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

@interface GearboyApplicationDelegate : NSObject <NSApplicationDelegate>
{
    id m_delegate;
    NSMenu* m_dock_menu;
}

- (instancetype)initWithDelegate:(id)delegate;
- (id)originalDelegate;

@end

@implementation GearboyApplicationDelegate

- (instancetype)initWithDelegate:(id)delegate
{
    self = [super init];
    if (self)
    {
        m_delegate = [delegate retain];
        m_dock_menu = [[NSMenu alloc] initWithTitle:@""];
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:@"New Window"
                                                     action:@selector(launchNewInstance:)
                                              keyEquivalent:@""];
        [item setTarget:self];
        [m_dock_menu addItem:item];
        [item release];
    }
    return self;
}

- (id)originalDelegate
{
    return m_delegate;
}

- (NSMenu*)applicationDockMenu:(NSApplication*)sender
{
    return m_dock_menu;
}

- (void)launchNewInstance:(id)sender
{
    NSURL* app_url = [[NSBundle mainBundle] bundleURL];
    NSWorkspaceOpenConfiguration* config = [NSWorkspaceOpenConfiguration configuration];
    config.createsNewApplicationInstance = YES;
    config.activates = YES;

    [[NSWorkspace sharedWorkspace] openApplicationAtURL:app_url
                                          configuration:config
                                      completionHandler:^(NSRunningApplication*, NSError* error)
    {
        if (error)
            NSLog(@"Failed to launch new Gearboy window: %@", error);
    }];
}

- (BOOL)respondsToSelector:(SEL)selector
{
    return [super respondsToSelector:selector] || [m_delegate respondsToSelector:selector];
}

- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector
{
    NSMethodSignature* signature = [super methodSignatureForSelector:selector];
    if (!signature)
        signature = [m_delegate methodSignatureForSelector:selector];
    return signature;
}

- (void)forwardInvocation:(NSInvocation*)invocation
{
    if ([m_delegate respondsToSelector:[invocation selector]])
        [invocation invokeWithTarget:m_delegate];
    else
        [super forwardInvocation:invocation];
}

- (void)dealloc
{
    [m_dock_menu release];
    [m_delegate release];
    [super dealloc];
}

@end

static GearboyApplicationDelegate* gearboy_application_delegate = nil;

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

extern "C" void macos_refocus_window(void* nswindow)
{
    NSWindow* win = (__bridge NSWindow*)nswindow;
    if (!win)
        return;

    if (@available(macOS 14.0, *))
    {
        [NSApp activate];
    }
    else
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [NSApp activateIgnoringOtherApps:YES];
#pragma clang diagnostic pop
    }

    [win makeKeyAndOrderFront:nil];
}

extern "C" void macos_install_dock_menu(void)
{
    if (gearboy_application_delegate || ![[[[NSBundle mainBundle] bundleURL] pathExtension] isEqualToString:@"app"])
        return;

    id delegate = [NSApp delegate];
    if (!delegate)
        return;

    gearboy_application_delegate = [[GearboyApplicationDelegate alloc] initWithDelegate:delegate];
    [NSApp setDelegate:gearboy_application_delegate];
}

extern "C" void macos_remove_dock_menu(void)
{
    if (!gearboy_application_delegate)
        return;

    if ([NSApp delegate] == gearboy_application_delegate)
        [NSApp setDelegate:[gearboy_application_delegate originalDelegate]];

    [gearboy_application_delegate release];
    gearboy_application_delegate = nil;
}

extern "C" void macos_launch_new_instance(void)
{
    if (gearboy_application_delegate)
        [gearboy_application_delegate launchNewInstance:nil];
}
