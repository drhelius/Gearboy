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

#import "GLViewController.h"
#import <MediaPlayer/MediaPlayer.h>

@interface GLViewController ()

@end

@implementation GLViewController

@synthesize context = _context;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        self.theEmulator = [[Emulator alloc]init];
        if ([[UIDevice currentDevice] userInterfaceIdiom] != UIUserInterfaceIdiomPhone)
        {
            self.view.hidden = YES;
        }
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    CGFloat scale;
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        scale=[[UIScreen mainScreen] scale];
    } else {
        scale=1; 
    }
    GLKView *view = (GLKView *)self.view;
    self.theEmulator.glview = view;
    BOOL retina, iPad;
    retina = (scale != 1);
    
    int multiplier = 0;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
    {
        iPad = NO;
        if (retina)
        {
            multiplier = 3;
            view.frame = CGRectMake(40, 28, 80 * multiplier, 72 * multiplier);
        }
        else
        {
            multiplier = 4;
            view.frame = CGRectMake(0, 0, 80 * multiplier, 72 * multiplier);
        }
    }
    else
    {
        iPad = YES;
        if (retina)
        {
            multiplier = 5;
            view.frame = CGRectMake(187, 53, 80 * multiplier, 72 * multiplier);
        }
        else
        {
            multiplier = 4;
            view.frame = CGRectMake(222, 82, 80 * multiplier, 72 * multiplier);
        }
    }
    
    self.theEmulator.multiplier = multiplier * (retina ? 2 : 1);
    self.theEmulator.retina = retina;
    self.theEmulator.iPad = iPad;
}

- (void)loadRomWithName: (NSString*) name
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentsDirectoryPath = [paths objectAtIndex:0];
    
    NSString* path = [NSString stringWithFormat:@"%@/%@", documentsDirectoryPath, name];
    
    [self.theEmulator loadRomWithPath:path];
    
    self.view.hidden = NO;
    self.paused = NO;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return (interfaceOrientation == UIInterfaceOrientationPortrait) || (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
}

- (void)update
{
    [self.theEmulator update];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    [self.theEmulator draw];
}

-(void) releaseContext
{
    [self.theEmulator shutdownGL];
    self.context = nil;
    GLKView *view = (GLKView *)self.view;
    view.context = nil;
    [EAGLContext setCurrentContext:nil];
}

-(void) acquireContext
{
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    [EAGLContext setCurrentContext:self.context];
    [self.theEmulator initGL];
    
    if ([[MPMusicPlayerController iPodMusicPlayer] playbackState] == MPMusicPlaybackStatePlaying)
        [self.theEmulator setAudio:NO];
    else
    {
        [self.theEmulator setAudio:YES];
        [self.theEmulator resetAudio];
    }
}

@end
