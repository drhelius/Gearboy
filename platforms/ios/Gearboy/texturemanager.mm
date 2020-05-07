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

#include "texturemanager.h"
#import "../../../src/gearboy.h"
#include <OpenGLES/ES1/glext.h>

//////////////////////////
//////////////////////////

TextureManager::TextureManager()
{
}

//////////////////////////
//////////////////////////

TextureManager::~TextureManager()
{
    UnloadAll();
}

//////////////////////////
//////////////////////////

bool TextureManager::LoadTexture(Texture* pTexture, bool mipmaps)
{
    Log("+++ TextureManager::LoadTexture Loading texture: %s\n", pTexture->m_strName);

    char* ind = strrchr(pTexture->m_strName, '/');
    char* szName = ind + 1;
    char szPath[256] = {0};
    strncpy(szPath, pTexture->m_strName, ind - pTexture->m_strName);
    szPath[ind - pTexture->m_strName] = 0;

    NSString * OCname = [NSString stringWithCString : szName encoding : [NSString defaultCStringEncoding]];
    NSString * OCpath = [NSString stringWithCString : szPath encoding : [NSString defaultCStringEncoding]];

    NSString * RSCpath = [[NSBundle mainBundle] pathForResource : OCname ofType : @"pvr" inDirectory : OCpath];

    FILE* pFile = fopen([RSCpath cStringUsingEncoding : 1], "r");

    if (pFile != NULL)
    {
        pTexture->m_bIsCompressed = true;

        fseek(pFile, 0, SEEK_END);
        int len = (int)ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        GLubyte* pBuffer = new GLubyte[len];
        fread(pBuffer, sizeof(GLubyte), len, pFile);

        fclose(pFile);

        int size = 0;
        GLenum internalformat = 0;

        for (int i = 16; i <= 1024; i *= 2)
        {
            if (((i * i) / 2) == len)
            {
                size = i;
                internalformat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
                break;
            }

            if (((i * i) / 4) == len)
            {
                size = i;
                internalformat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
                break;
            }
        }

        if (size != 0)
        {
            pTexture->m_iWidth = size;
            pTexture->m_iHeight = size;

            glGenTextures(1, &pTexture->m_theTexture);
            glBindTexture(GL_TEXTURE_2D, pTexture->m_theTexture);
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalformat, size, size, 0, len, pBuffer);
        }
        else
        {
            Log("@@@ TextureManager::LoadTexture PVR incorrect size: %s.pvr\n", pTexture->m_strName);
            SafeDeleteArray(pBuffer);
            return false;
        }
        
        SafeDeleteArray(pBuffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Log("+++ TextureManager::LoadTexture size %dx%d\n", pTexture->m_iWidth, pTexture->m_iHeight);
        Log("+++ TextureManager::LoadTexture PVR texture loaded: %s.pvr\n", pTexture->m_strName);

        return true;
    }
    else
    {
        Log("@@@ TextureManager::LoadTexture Unable to load PVR: %s.pvr\n", pTexture->m_strName);

        pTexture->m_bIsCompressed = false;

        char szPathPNG[256] = {0};
		strcpy(szPathPNG, "/");
        strcat(szPathPNG, pTexture->m_strName);
        strcat(szPathPNG, ".png");

        CGImageRef spriteImage;
        CGContextRef spriteContext;
        GLubyte *spriteData;
        int width, height;
		
		char pathSize[2048] = {0};
		
		
		NSString* path =[[NSBundle mainBundle]resourcePath];	
		
		[path getCString:pathSize maxLength:2048 encoding:NSUTF8StringEncoding];
		
		strcat(pathSize, szPathPNG);			
		
		// Creates a Core Graphics image from an image file
		spriteImage = [UIImage imageWithContentsOfFile : [NSString stringWithUTF8String:pathSize]].CGImage;	
       

        if (spriteImage)
        {
            // Get the width and height of the image
            pTexture->m_iWidth = width = (int)CGImageGetWidth(spriteImage);
            pTexture->m_iHeight = height = (int)CGImageGetHeight(spriteImage);
            // Texture dimensions must be a power of 2. If you write an application that allows users to supply an image,
            // you'll want to add code that checks the dimensions and takes appropriate action if they are not a power of 2.

            // Allocated memory needed for the bitmap context
            spriteData = (GLubyte *) calloc(width * height * 4, 1);
            // Uses the bitmatp creation function provided by the Core Graphics framework.
            spriteContext = CGBitmapContextCreate(spriteData, width, height, 8, width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
            // After you create the context, you can draw the sprite image to the context.
            CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, (CGFloat) width, (CGFloat) height), spriteImage);
            // You don't need the context at this point, so you need to release it to avoid memory leaks.
            CGContextRelease(spriteContext);

            // Use OpenGL ES to generate a name for the texture.
            glGenTextures(1, &pTexture->m_theTexture);
            // Bind the texture name.
            glBindTexture(GL_TEXTURE_2D, pTexture->m_theTexture);
            // Speidfy a 2D texture image, provideing the a pointer to the image data in memory          
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
            // Release the image data

            if (mipmaps)
            {
                Log("+++ TextureManager::LoadTexture generating mipmaps...\n");

                GLubyte *prevImage = 0;
                GLubyte *newImage = 0;

                int level = 1;
                prevImage = &spriteData[0];

                while (width > 1 && height > 1)
                {
                    int newWidth, newHeight;

                    // Generate the next mipmap level
                    GenMipMap2D(prevImage, &newImage, width, height,
                            &newWidth, &newHeight, level);

                    // Load the mipmap level
                    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
                            newWidth, newHeight, 0, GL_RGBA,
                            GL_UNSIGNED_BYTE, newImage);

                    // Free the previous image
                    free(prevImage);

                    // Set the previous image for the next iteration
                    prevImage = newImage;
                    level++;

                    // Half the width and height
                    width = newWidth;
                    height = newHeight;
                }

                free(newImage);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            }
            else
            {
                free(spriteData);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Log("+++ TextureManager::LoadTexture size %dx%d\n", pTexture->m_iWidth, pTexture->m_iHeight);
            Log("+++ TextureManager::LoadTexture PNG texture loaded: %s.png\n", pTexture->m_strName);
            return true;
        }
        else
        {
            Log("@@@ TextureManager::LoadTexture Unable to load texture: %s\n", pTexture->m_strName);
            Log("@@@ TextureManager::LoadTexture Defaulting to missing texture\n");

            pTexture->m_bIsCompressed = false;

            CGImageRef spriteImage;
            CGContextRef spriteContext;
            GLubyte *spriteData;
            int width, height;

            // Creates a Core Graphics image from an image file
            spriteImage = [UIImage imageNamed : [NSString stringWithCString : "missing_texture.png" encoding : [NSString defaultCStringEncoding]]].CGImage;

            if (spriteImage)
            {
                // Get the width and height of the image
                pTexture->m_iWidth = width = (int)CGImageGetWidth(spriteImage);
                pTexture->m_iHeight = height = (int)CGImageGetHeight(spriteImage);
                // Texture dimensions must be a power of 2. If you write an application that allows users to supply an image,
                // you'll want to add code that checks the dimensions and takes appropriate action if they are not a power of 2.

                // Allocated memory needed for the bitmap context
                spriteData = (GLubyte *) calloc(width * height * 4, 1);
                // Uses the bitmatp creation function provided by the Core Graphics framework.
                spriteContext = CGBitmapContextCreate(spriteData, width, height, 8, width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
                // After you create the context, you can draw the sprite image to the context.
                CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, (CGFloat) width, (CGFloat) height), spriteImage);
                // You don't need the context at this point, so you need to release it to avoid memory leaks.
                CGContextRelease(spriteContext);

                // Use OpenGL ES to generate a name for the texture.
                glGenTextures(1, &pTexture->m_theTexture);
                // Bind the texture name.
                glBindTexture(GL_TEXTURE_2D, pTexture->m_theTexture);
                // Speidfy a 2D texture image, provideing the a pointer to the image data in memory
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
                // Release the image data

                free(spriteData);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                Log("+++ TextureManager::LoadTexture missing_texture.png loaded\n");
                return true;
            }
            else
            {
                Log("@@@ TextureManager::LoadTexture Unable to load default missing texture: missing_texture.png\n");

                return false;
            }
        }
    }
}

//////////////////////////
//////////////////////////

GLboolean TextureManager::GenMipMap2D(GLubyte *src, GLubyte **dst, int srcWidth, int srcHeight, int *dstWidth, int *dstHeight, int level)
{
    int x, y;
    int texelSize = 4;

    *dstWidth = srcWidth / 2;
    if (*dstWidth <= 0)
        *dstWidth = 1;

    *dstHeight = srcHeight / 2;
    if (*dstHeight <= 0)
        *dstHeight = 1;

    *dst = (GLubyte*) malloc(sizeof(GLubyte) * texelSize * (*dstWidth) * (*dstHeight));
    if (*dst == NULL)
        return GL_FALSE;

    for (y = 0; y < *dstHeight; y++)
    {
        for (x = 0; x < *dstWidth; x++)
        {
            int srcIndex[4];
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
            int sample;

            // Compute the offsets for 2x2 grid of pixels in previous
            // image to perform box filter
            srcIndex[0] = (((y * 2) * srcWidth) + (x * 2)) * texelSize;
            srcIndex[1] = (((y * 2) * srcWidth) + (x * 2 + 1)) * texelSize;
            srcIndex[2] = ((((y * 2) + 1) * srcWidth) + (x * 2)) * texelSize;
            srcIndex[3] = ((((y * 2) + 1) * srcWidth) + (x * 2 + 1)) * texelSize;

            // Sum all pixels
            for (sample = 0; sample < 4; sample++)
            {
                r += src[srcIndex[sample]];
                g += src[srcIndex[sample] + 1];
                b += src[srcIndex[sample] + 2];
                a += src[srcIndex[sample] + 3];
            }

            // Average results
            r /= 4.0;
            g /= 4.0;
            b /= 4.0;
            a /= 4.0;

            // Store resulting pixels
            (*dst)[ (y * (*dstWidth) + x) * texelSize ] = (GLubyte) (r);
            (*dst)[ (y * (*dstWidth) + x) * texelSize + 1] = (GLubyte) (g);
            (*dst)[ (y * (*dstWidth) + x) * texelSize + 2] = (GLubyte) (b);
            (*dst)[ (y * (*dstWidth) + x) * texelSize + 3] = (GLubyte) (a);
        }
    }

    Log("+++ TextureManager::GenMipMap2D level %d: %dx%d\n", level, *dstWidth, *dstHeight);

    return GL_TRUE;
}

//////////////////////////
//////////////////////////

Texture* TextureManager::GetTexture(const char* strTextureName, bool mipmaps)
{
    Log("+++ TextureManager::GetTexture Searching for %s\n", strTextureName);

    std::string stringTextureName(strTextureName);

    TTextureMapIterator lowerBound = m_TextureMap.lower_bound(stringTextureName);

    ///--- ya estaba
    if (lowerBound != m_TextureMap.end() &&
            !(m_TextureMap.key_comp()(stringTextureName, lowerBound->first)))
    {
        Log("+++ TextureManager::GetTexture Already in use\n");

        return lowerBound->second;
    }
        ///--- no estaba
    else
    {
        Texture* temp = new Texture();
        strcpy(temp->m_strName, strTextureName);

        if (LoadTexture(temp, mipmaps))
        {
            TTextureMapPair insertPair(stringTextureName, temp);

            TTextureMapIterator result = m_TextureMap.insert(lowerBound, insertPair);

            if (result->second)
            {
                ////--- se ha insertado la textura
                Log("+++ TextureManager::GetTexture New texture inserted\n");
                return temp;
            }
            else
            {
                ///--- ya exist√≠a??? nunca debe entrar aqui
                Log("@@@ TextureManager::GetTexture FATAL ERROR @@@\n");
            }
        }
        else
        {
            ///--- no se pudo cargar la textura
            Log("@@@ TextureManager::GetTexture Unable to load texture %s\n", strTextureName);
        }

        SafeDelete(temp);
        return NULL;
    }
}

//////////////////////////
//////////////////////////

bool TextureManager::UnloadTexture(const char* strTextureName)
{
    std::string stringTextureName(strTextureName);

    TTextureMapIterator itor = m_TextureMap.find(stringTextureName);

    ///--- estaba
    if (itor != m_TextureMap.end())
    {
        Log("+++ TextureManager::UnloadTexture Deleting texture: %s\n", itor->second->m_strName);

        glDeleteTextures(1, &itor->second->m_theTexture);

        SafeDelete(itor->second);

        m_TextureMap.erase(itor);

        return true;
    }
        ///--- no estaba
    else
    {
        Log("@@@ TextureManager::UnloadTexture The texture was not found\n");
        return false;
    }
}

//////////////////////////
//////////////////////////

bool TextureManager::UnloadTexture(Texture* pTexture)
{
    return UnloadTexture(pTexture->m_strName);
}

//////////////////////////
//////////////////////////

int TextureManager::UnloadAll(void)
{
    for (TTextureMapIterator i = m_TextureMap.begin(); i != m_TextureMap.end(); i++)
    {
        Log("+++ TextureManager::UnloadAll Deleting texture: %s\n", i->second->m_strName);

        glDeleteTextures(1, &i->second->m_theTexture);

        SafeDelete(i->second);
    }

    m_TextureMap.clear();

    return 0;
}

