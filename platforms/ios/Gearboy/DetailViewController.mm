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
    
    [self.view addSubview:self.theGLViewController.view];
    
    /*
     CGRect screenBounds = [[UIScreen mainScreen] bounds];
     if (screenBounds.size.height == 568)
     {
     // 4-inch screen (iPhone 5)
     for (UIView* subview in self.view.subviews)
     {
     if ([subview isKindOfClass:[UIImageView class]])
     {
     UIImageView* background = (UIImageView*)subview;
     background.image = [UIImage imageNamed:@"gb_1008.jpg"];
     }
     }
     }*/
    
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
