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
    
    UIBarButtonItem *menu = [[UIBarButtonItem alloc] initWithTitle:@"Menu" style:UIBarButtonItemStylePlain target:self action:@selector(menuButtonPressed)];
    
    self.navigationItem.rightBarButtonItems = @[menu];

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
            imageName = @"gb_1206.jpg";
            break;
        }
        case 736:
        {
            // iPhone 6 Plus
            scrW = 414;
            scrH = 672;
            imageName = @"gb_2016.jpg";
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

-(void)menuButtonPressed {
    
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Menu"
                                                            message:@"Menu options."
                                                            preferredStyle:UIAlertControllerStyleActionSheet];
    
    UIAlertAction *save = [UIAlertAction actionWithTitle:@"Save"
                                                style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
                                                    [self saveState];
                                                }];
    UIAlertAction *load = [UIAlertAction actionWithTitle:@"Load"
                                                style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
                                                    [self loadState];
                                                }];
    UIAlertAction *shareROM = [UIAlertAction actionWithTitle:@"Share ROM"
                                                style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
                                                    [self airdropROM];
                                                }];
    UIAlertAction *shareSaveFile = [UIAlertAction actionWithTitle:@"Share Save File"
                                                style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {
                                                    [self airdropSaveFile];
                                                }];
    UIAlertAction *cancel = [UIAlertAction actionWithTitle:@"Cancel"
                                                            style:UIAlertActionStyleCancel handler:NULL];
    [alert addAction:save];
    [alert addAction:load];
    [alert addAction:shareROM];
    [alert addAction:shareSaveFile];
    [alert addAction:cancel];
    
    [self presentViewController:alert animated:YES completion:nil];
}

-(void)saveState
{
    [self.theGLViewController.theEmulator saveState];
}

-(void)loadState
{
     [self.theGLViewController.theEmulator loadState];
}

-(void)airdropROM
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentsDirectoryPath = [paths objectAtIndex:0];
    NSString* romPath = [NSString stringWithFormat:@"%@/%@", documentsDirectoryPath, self.detailItem];
    NSURL* romFile = [NSURL fileURLWithPath:romPath];
    NSArray* objectsToAirdrop = @[romFile];
    UIActivityViewController* controller = [[UIActivityViewController alloc] initWithActivityItems:objectsToAirdrop applicationActivities:nil];
    
    NSArray* excludedActivities = @[UIActivityTypePostToTwitter, UIActivityTypePostToFacebook,
                                    UIActivityTypePostToWeibo,
                                    UIActivityTypeMessage, UIActivityTypeMail,
                                    UIActivityTypePrint, UIActivityTypeCopyToPasteboard,
                                    UIActivityTypeAssignToContact, UIActivityTypeSaveToCameraRoll,
                                    UIActivityTypeAddToReadingList, UIActivityTypePostToFlickr,
                                    UIActivityTypePostToVimeo, UIActivityTypePostToTencentWeibo];
    controller.excludedActivityTypes = excludedActivities;
    
    [self presentViewController:controller animated:YES completion:nil];
}

-(void)airdropSaveFile
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentsDirectoryPath = [paths objectAtIndex:0];
    NSString* saveFilename = [self.detailItem stringByReplacingOccurrencesOfString:@".gbc" withString:@".sav"];
    NSString* saveFilePath = [NSString stringWithFormat:@"%@/%@", documentsDirectoryPath, saveFilename];
    NSURL* saveFile = [NSURL fileURLWithPath:saveFilePath];
    NSArray* objectsToAirdrop = @[saveFile];
    UIActivityViewController* controller = [[UIActivityViewController alloc] initWithActivityItems:objectsToAirdrop applicationActivities:nil];
    
    NSArray* excludedActivities = @[UIActivityTypePostToTwitter, UIActivityTypePostToFacebook,
                                    UIActivityTypePostToWeibo,
                                    UIActivityTypeMessage, UIActivityTypeMail,
                                    UIActivityTypePrint, UIActivityTypeCopyToPasteboard,
                                    UIActivityTypeAssignToContact, UIActivityTypeSaveToCameraRoll,
                                    UIActivityTypeAddToReadingList, UIActivityTypePostToFlickr,
                                    UIActivityTypePostToVimeo, UIActivityTypePostToTencentWeibo];
    controller.excludedActivityTypes = excludedActivities;
    
    [self presentViewController:controller animated:YES completion:nil];
}

@end
