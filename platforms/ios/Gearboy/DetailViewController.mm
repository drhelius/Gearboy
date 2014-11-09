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

#import "DetailViewController.h"
#include "inputmanager.h"

@interface DetailViewController ()

@end

@implementation DetailViewController

#pragma mark - Managing the detail item

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.multipleTouchEnabled = YES;
    
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    
    int scrW = 480;
    int scrH = 640;
    NSString* imageName = @"gb_832.jpg";
    
    switch ((int)screenBounds.size.height)
    {
        case 480:
        {
            // iPhone 4
            scrW = 320;
            scrH = 416;
            imageName = @"gb_832.jpg";
            break;
        }
        case 568:
        {
            // iPhone 5
            scrW = 320;
            scrH = 504;
            imageName = @"gb_1008.jpg";
            break;
        }
        case 667:
        {
            // iPhone 6
            scrW = 375;
            scrH = 603;
            imageName = @"gb_1008.jpg";
            break;
        }
        case 736:
        {
            // iPhone 6 Plus
            scrW = 414;
            scrH = 672;
            imageName = @"gb_1008.jpg";
            break;
        }
        case 1024:
        {
            scrW = 768;
            scrH = 960;
            if ([[UIScreen mainScreen] scale] != 1)
            {
                // iPad Air
                imageName = @"gb_ipad_1920.jpg";
            }
            else
            {
                // iPad 2
                imageName = @"gb_ipad_960.jpg";
            }
            break;
        }
    }
    
    UIImageView *imgView = [[UIImageView alloc] init];
    imgView.image = [UIImage imageNamed:imageName];
    imgView.frame = CGRectMake(0, 0, scrW, scrH);
    
    [self.view addSubview:imgView];
    [self.view addSubview:self.theGLViewController.view];
    
    if (self.detailItem)
    {
        [self.theGLViewController loadRomWithName:self.detailItem];
    }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
    {
        return (interfaceOrientation == UIInterfaceOrientationPortrait) || (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
    }
    else
    {
        return (interfaceOrientation == UIInterfaceOrientationPortrait) || (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
    }
}

-(void) _handleTouch : (UITouch *) touch
{
    InputManager::Instance().HandleTouch(touch, self.view);
}

-(void) touchesBegan : (NSSet *) touches withEvent : (UIEvent *) event
{
    for (UITouch *touch in touches)
    {
        [self _handleTouch : touch];
    }
}

-(void) touchesMoved : (NSSet *) touches withEvent : (UIEvent *) event
{
    for (UITouch *touch in touches)
    {
        [self _handleTouch : touch];
    }
}

-(void) touchesEnded : (NSSet *) touches withEvent : (UIEvent *) event
{
    for (UITouch *touch in touches)
    {
        [self _handleTouch : touch];
    }
}

-(void) touchesCancelled : (NSSet *) touches withEvent : (UIEvent *) event
{
    for (UITouch *touch in touches)
    {
        [self _handleTouch : touch];
    }
}

@end
