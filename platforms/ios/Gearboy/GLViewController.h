//
//  GLViewController.h
//  Gearboy
//
//  Created by Ignacio Sánchez Ginés on 09/09/12.
//  Copyright (c) 2012 Ignacio Sánchez Ginés. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "Emulator.h"

@interface GLViewController : GLKViewController
{
    Emulator* theEmulator;
}

@property (strong, nonatomic) EAGLContext *context;

@end
