/*
The OGL.h file is here to create a simple class to control our use of OpenGLES2.0 on the Raspberry
It contains some start up code to get OpenGLES up and working in the function InitOGL
It also contains some simple swap functions to make the screen visible
*/

#pragma once
// these headers give the code here access to concepts
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include "AssetManager.h"

// be careful only to name the flage and give its value since the compiler injects this into the code so adding a //comment can end the line 
#define  FastUpdate TRUE


// we can create new types of C++ "types" using a struct and a typedef

typedef struct simpleVec2 // <<< we inform C++ we are defining a new type, its a structure called simpleVec2
{
	float xValue; 	// new types can only consist of already known types though,
	float yValue; 	// which is fine as we only really want to be able to store 2 floats

}  simpleVec2;  // confirm to C++ that this is our new type

typedef struct simpleintVec2 // <<< we inform C++ we are defining a new type, its a structure called simpleVec2
{
	unsigned int xValue;  	// new types can only consist of already known types though,
	unsigned int yValue;   	// which is fine as we only really want to be able to store 2 floats
	
}  simpleintVec2;   // confirm to C++ that this is our new type


// this header is very important for Raspberry Machines, it allows us to access initialisation and size functions for our Broadcomm GPU
#include "bcm_host.h"
#define FALSE 0
#define TRUE 1
#define BLUEMASK	(0x00ff0000)
#define GREENMASK	(0x0000ff00) 
#define REDMASK		(0x000000ff)
#define ALPHAMASK	(0xff000000)

class OGL
{
public:
	OGL();
	~OGL();
	
	// our Pi needs to have an OGLES context, a kind of buffer area, where info on the set up is stored, this will help us access it
	// we will keep track of our context and some of the other things we need.
	typedef struct Target_State
	{
		uint32_t width;  
		uint32_t height;

		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;
							//Although the Raspberry Pi uses a "standed" EGL contect to render to, we draw to a Broadcomm style window 
							//using a system called Dispmanx, here we can keep track of it.
		EGL_DISPMANX_WINDOW_T nativewindow;
	} Target_State;

	
	GLuint LoadShader(GLenum type, const char *shaderSrc);
	int Init();
	void init_EGL(Target_State *state, int width, int height);
	GLuint CreateTexture2D(int width, int height,  char* TheData); // this is a handy routine for making textures
	

/*Now that we have created a type called a Target_State, we can use it in code.	We can create it here */
	
	Target_State state; 
	
	GLint programObject; // having only one global PO is limiting, but for now it will make sense.
	GLint FastProgramObject; // now we can see why it is limiting, we are going to draw somethings with a different PO
	GLint TextProgramObject; // a PO to render text
	GLint ProgramObject3D; // a much more interesting one
	GLint ProgramObjectSprites; // allows for rotation and other effects
	GLuint textureID;
	float OrthoMatrix[4][4]; // this is a complex concept and best done with libs later.
	
	void setOrthoMatrix( const float &bottom,
		const float &top,
		const float &left,
		const float &right, 
		const float &near,
		const float &far);
	
	AssetManager TheAssetManager;   // this will be the central repository of our assets
	
		
};