#pragma once
#include <string> // we're going to use strings so need the header
#include "OGL.h"
#include "stb_image.h"
#undef countof
#include <glm/glm.hpp>
//#include "../glm/glm/gtc/matrix_transform.hpp"
#undef countof


class Game; // let SimpleObj class know there is a class called game, we can't include it because it also references SimpleOBJ


// This is something new, lets define directions
typedef	 enum Direction
{
	Up    = 0,
	Right,
	Down,
	Left,
	Homing
} Direction;

class SimpleObj
{
public:
	SimpleObj(); //standard constructor
	virtual ~SimpleObj(); // becuase this now is the base class for many polymorph types, we will ensure that their destructor is called by making this virtual


//list the functions we want to have (called methods in C++)	

	static void SetGraphics3D(Game* G);  // we'll call this one time to set up the "3d sprites" shader values, they are all the same so we want to avoid duplicating effort.
	virtual void Update();  // By making these virtual, it allows a class which inherets SimpleObj to replace these functions with their own versions
	virtual void Draw(); // that will be even more obvious when we decide to have different draw systems for different sprites
	virtual bool Update(Game*); // most of our autononous objects will use this upadate
// we will add this here so every object can test if it hit another object	
	bool DidIGetHit(SimpleObj* Obj2);
	
// list the variables we want our instances to have (called members in C++)	
	float Xpos, Ypos, TestYpos; // test Ypos is a flipped version of Ypos we will use
	float Xspeed, Yspeed;
	float ScaleX;	// this is just a fun variable since our actual texture is quite large, and already shrunk but the vertex offsets
	float ScaleY;
	
	float ScreenX, ScreenY; // after a scroll we might need to review the Screen position
// when we load our images we can store the width and height here it will help with collison	
	float width;
	float height;
	std::string m_MyName;
	
	GLint TextureID;   		// each tile now has its own

	simpleVec2	XYOffsets; // use this to shift positional data (quite a hacky way to do this, but we will soon show a better way)
	
// the static command tells us that no matter how many instances of SimpleObj we create they are only going to have 1 version of these variables
// so all instances will share then, this is powerful and useful, but can also complicate things we we decide to have different values
// these all relate to how our shader works, and in this case that seems fine, for a default shader, if we want to have a derived object with
// a different shader we can simply ensure that we have non static instances in the derived classes and make sure   derived class uses those
	
	static GLint ProgramObject; //Objects need this and it is now changable
	// Get the attribute locations, we'll store these in values we can call handles.
	static GLint positionLoc;
	static GLint texCoordLoc;
	
	// if we want to position it somewhere we need to ask for the handle for the position uniform and other unfirms
	static GLint positionUniformLoc;
	static GLint samplerLoc;  //= glGetUniformLocation(ProgramObject, "s_texture");
	static GLint ScreenCoordLoc;  //= glGetUniformLocation(ProgramObject, "u_Screensize");
	static GLint ScaleLoc;  //= glGetUniformLocation(ProgramObject, "u_Scale"); 
	static GLuint VBO; // since we only have one type of object at the moment (a quad) we can make do with one vbo
	static GLuint OffsetsLoc;
	static GLuint MVPLoc;
// at some point most graphic objects are going to need to know where the OGL class is, since it allows us access
// to some useful tooks like creating textures
	static OGL* TheOGLInstance; // we've now made this static
	static Game* GameLoc;
	glm::mat4 Model; 
	Direction Dir;

	
};