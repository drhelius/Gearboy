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

#import <GLKit/GLKit.h>
#import "Emulator.h"

@implementation Emulator

-(id)init
{
    if (self = [super init])
    {
        theGearboyCore = new GearboyCore();
        theGearboyCore->Init();
        theGearboyCore->SetSoundSampleRate(44100);
        initialized = NO;
        width = 160;
        height = 144;
        theFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
        theTexture = new GB_Color[256 * 256];
        
        for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
        {
            for (int x = 0; x < GAMEBOY_WIDTH; ++x)
            {
                int pixel = (y * GAMEBOY_WIDTH) + x;
                theFrameBuffer[pixel].red = theFrameBuffer[pixel].green =
                theFrameBuffer[pixel].blue = theFrameBuffer[pixel].alpha = 255;
            }
        }
        
        for (int y = 0; y < 256; ++y)
        {
            for (int x = 0; x < 256; ++x)
            {
                int pixel = (y * 256) + x;
                theTexture[pixel].red = theTexture[pixel].green =
                theTexture[pixel].blue = theTexture[pixel].alpha = 0;
            }
        }
        
        NSString* path = [[NSBundle mainBundle] pathForResource:@"kwirk" ofType:@"gb"];
        
        theGearboyCore->LoadROM([path UTF8String], false);
    }
    return self;
}

-(void)dealloc
{
    SafeDeleteArray(theTexture);
    SafeDeleteArray(theFrameBuffer);
    SafeDelete(theGearboyCore);
}

-(void)update
{
    theGearboyCore->RunToVBlank(theFrameBuffer);
    
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            theTexture[(y * 256) + x] = theFrameBuffer[(y * GAMEBOY_WIDTH) + x];
        }
    }
}

-(void)draw
{
    if (!initialized)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glEnable(GL_TEXTURE_2D);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof(0.0f, width, 0.0f, height, -100.0f, 100.0f);
        glMatrixMode(GL_MODELVIEW);
        glViewport(0, 0, width, height);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        initialized = YES;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
	GLfloat box[] = {0,(float)height,1, (float)width,(float)height,1, 0,0,1, (float)width,0,1};
	GLfloat tex[] = {0,0, 160.0f / 256.0f,0, 0, 144.0f / 256.0f, 160.0f / 256.0f,144.0f / 256.0f};
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
    
	glVertexPointer(3, GL_FLOAT, 0,box);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
    
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

-(void)loadRomWithPath: (NSString *)filePath andForceDMG: (BOOL) forceDMG
{
    theGearboyCore->LoadROM([filePath UTF8String], forceDMG);
}

-(void)keyPressed: (Gameboy_Keys)key
{
    theGearboyCore->KeyPressed(key);
}

-(void)keyReleased: (Gameboy_Keys)key
{
    theGearboyCore->KeyReleased(key);
}

-(void)pause
{
    theGearboyCore->Pause(true);
}

-(void)resume
{
    theGearboyCore->Pause(false);
}

-(BOOL)paused
{
    return theGearboyCore->IsPaused();
}

-(void)reset: (BOOL)forceDMG
{
    theGearboyCore->ResetROM(forceDMG);
}

@end
