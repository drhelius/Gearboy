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

#import <Foundation/Foundation.h>
#import "../../src/gearboy.h"

@interface Emulator : NSObject
{
    GearboyCore* theGearboyCore;
    BOOL initialized;
    int width, height;
    GB_Color* theFrameBuffer;
    GB_Color* theTexture;
}

-(void)update;
-(void)draw;
-(void)loadRomWithPath: (NSString *)filePath andForceDMG: (BOOL) forceDMG;
-(void)keyPressed: (Gameboy_Keys)key;
-(void)keyReleased: (Gameboy_Keys)key;
-(void)pause;
-(void)resume;
-(BOOL)paused;
-(void)reset: (BOOL)forceDMG;

@end
