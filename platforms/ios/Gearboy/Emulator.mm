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
#include "inputmanager.h"

const float kGB_Width = 160.0f;
const float kGB_Height = 144.0f;
const float kGB_TexWidth = kGB_Width / 256.0f;
const float kGB_TexHeight = kGB_Height / 256.0f;
const GLfloat box[] = {0.0f, kGB_Height, 1.0f, kGB_Width,kGB_Height, 1.0f, 0.0f, 0.0f, 1.0f, kGB_Width, 0.0f, 1.0f};
const GLfloat tex[] = {0.0f, 0.0f, kGB_TexWidth, 0.0f, 0.0f, kGB_TexHeight, kGB_TexWidth, kGB_TexHeight};

@implementation Emulator

@synthesize context = _context;

-(id)init
{
    if (self = [super init])
    {
        theGearboyCore = new GearboyCore();
        theGearboyCore->Init();
        
        GB_Color color1;
        GB_Color color2;
        GB_Color color3;
        GB_Color color4;
    
        color1.alpha = 0;
        color1.red = 0xB8;
        color1.green = 0xC2;
        color1.blue = 0x66;
        color2.alpha = 0;
        color2.red = 0x7B;
        color2.green = 0x8A;
        color2.blue = 0x32;
        color3.alpha = 0;
        color3.red = 0x43;
        color3.green = 0x59;
        color3.blue = 0x1D;
        color4.alpha = 0;
        color4.red = 0x13;
        color4.green = 0x2C;
        color4.blue = 0x13;
        
        theGearboyCore->SetDMGPalette(color1, color2, color3, color4);
        
        theInput = new EmulatorInput(self);
        theInput->Init();
        initialized = NO;
        theFrameBuffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
        theTexture = new GB_Color[256 * 256];
        
        for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
        {
            for (int x = 0; x < GAMEBOY_WIDTH; ++x)
            {
                int pixel = (y * GAMEBOY_WIDTH) + x;
                theFrameBuffer[pixel].red = theFrameBuffer[pixel].green =
                theFrameBuffer[pixel].blue = theFrameBuffer[pixel].alpha = 0;
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
    InputManager::Instance().Update();
    
    theGearboyCore->RunToVBlank(theFrameBuffer);
    
    for (int y = 0; y < GAMEBOY_HEIGHT; ++y)
    {
        for (int x = 0; x < GAMEBOY_WIDTH; ++x)
        {
            theTexture[(y * 256) + x] = theFrameBuffer[(y * GAMEBOY_WIDTH) + x];
        }
    }
}

-(void)init
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, kGB_Width, 0.0f, kGB_Height, -100.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, box);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    initialized = YES;
}

-(void)renderMixFrames
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);

	
    
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_IntermediateFramebuffer);
    glBindTexture(GL_TEXTURE_2D, m_GBTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
    RenderQuad(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, m_AccumulationFramebuffer);
    glBindTexture(GL_TEXTURE_2D, m_IntermediateTexture);
    if (m_bFirstFrame)
    {
        m_bFirstFrame = false;
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, kMixFrameAlpha);
    }
    RenderQuad(GAMEBOY_WIDTH, GAMEBOY_HEIGHT);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, m_AccumulationTexture);
    if (m_bFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    RenderQuad(m_iWidth, m_iHeight);
}

-(void)renderQuadWithViewportWidth: (int)viewportWidth andHeight: (int)viewportHeight
{
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

-(void)setupTextureWithData: (GLvoid*) data
{
}

-(void)draw
{
    if (!initialized)
    {
        [self init];

    }
    
    [self renderMixFrames];
}

-(void)loadRomWithPath: (NSString *)filePath
{
    theGearboyCore->LoadROM([filePath UTF8String], false);
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

-(void)reset
{
    theGearboyCore->ResetROM(false);
}

@end
