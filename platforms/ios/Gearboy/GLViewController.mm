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
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    
    CGFloat scale;
    if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)]) {
        scale=[[UIScreen mainScreen] scale];
    } else {
        scale=1; 
    }
    
    BOOL retina;
    retina = (scale != 1);
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
    {
        if (retina)
        {
            view.frame = CGRectMake(40, 28, 80 * 3, 72 * 3);
        }
        else
        {
            view.frame = CGRectMake(0, 0, 80 * 4, 72 * 4);
        }
    }
    else
    {
        if (retina)
        {
            view.frame = CGRectMake(187, 53, 80 * 5, 72 * 5);
        }
        else
        {
            view.frame = CGRectMake(222, 82, 80 * 4, 72 * 4);
        }
    }
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

@end
