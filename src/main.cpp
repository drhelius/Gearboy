#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "gearboy.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

const int modifier = 4;

int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

void setupTexture()
{
    // Clear screen
    for(int y = 0; y < SCREEN_HEIGHT; ++y)         
        for(int x = 0; x < SCREEN_WIDTH; ++x)
            screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;
 
    // Create a texture 
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
 
    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
 
    // Enable textures
    glEnable(GL_TEXTURE_2D);
}
 
void updateTexture(const chip8& c8)
{     
    // Update pixels
    for(int y = 0; y < 32; ++y)               
        for(int x = 0; x < 64; ++x)
            if(c8.gfx[(y * 64) + x] == 0)
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;  // Disabled
            else 
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled
 
    // Update Texture
    glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
 
    glBegin( GL_QUADS );
        glTexCoord2d(0.0, 0.0);          glVertex2d(0.0,                      0.0);
        glTexCoord2d(1.0, 0.0);  glVertex2d(display_width, 0.0);
        glTexCoord2d(1.0, 1.0);  glVertex2d(display_width, display_height);
        glTexCoord2d(0.0, 1.0);  glVertex2d(0.0,                      display_height);
    glEnd();
}
 
void display()
{
    myChip8.emulateCycle();
 
    if(myChip8.drawFlag)
    {
        // Clear framebuffer
        glClear(GL_COLOR_BUFFER_BIT);
 
        // Draw pixels to texture
        updateTexture(myChip8);
 
        // Swap buffers!
        glutSwapBuffers();    
 
        // Processed frame
        myChip8.drawFlag = false;
        }
}
 
void reshape_window(GLsizei w, GLsizei h)
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);        
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
 
    // Resize quad
    display_width = w;
    display_height = h;
}


int main(int argc, char** argv)
{
    Core* gb = new Core();

    glutInit(&argc, argv);          
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
 
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(320, 320);
    glutCreateWindow();
 
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);
 
    setupTexture();
 
    glutMainLoop(); 
    
    SafeDelete(gb);
    
    return 0;
}


