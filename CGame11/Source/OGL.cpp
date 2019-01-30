#include  "OGL.h"
#include <assert.h> // assert is a way of making sure that things are what we expect and if not it will stop and tell us.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

/*  For beginners this might be a bit too much, but just include the file for now and use the OGL.h file when you need to use
	OpenGLES2.0 instructions. 
	
	In order to set up a screen, we will need to ask the video system to create a couple of buffer areas with 
	the right type of pixels and also indicate some of the hardware features we want to switch on. This is done using EGL libs.
	
	The process of setting up a graphic screen is a little technical but mainly we must create an EGLscreen
	which itself is optimised on RaspPi to use a special format our Broadcom GPU givs us for its display, all handled in 
	the bcm_host.h file so we can forget about it, but we do need to tell EGL we are using DISPMANX systems
	
	Once we have our EGL/DISMPANX surface and displays systems it means OpenGLES2.0 has something to work with and we can
	draw things using our shaders.
	For convenience we will set up a simple default shader, but normally we would have a few different ones for different
	effects.
	
	You can totally ignore the contents of this file until you feel you want to explore how to set things up, but do make 
	sure you have an #include OGL.h in any class that wants to play with OpenGL and that you double check that you create 
	an instance of OGL and call its init() method to set it all up of us.
	
	Also note this file makes use of fprintf instead of cout, its just a tidier way to format printed text.
	
*/

OGL::OGL() {};
OGL::~OGL() {};

// this is a simple definition of an attribute list to create an EGL window, there are many variations we could have
static const EGLint attribute_list[] =
{
// these 1st 4, let us define the size of our colour componants and from that the size of a pixel. 
// In this case 32bits made up of 8xR, 8xG,8xB and 8xA 
	EGL_RED_SIZE,
	8,
	EGL_GREEN_SIZE,
	8,
	EGL_BLUE_SIZE,
	8,
	EGL_ALPHA_SIZE,
	8,
	
//	EGL_SAMPLE_BUFFERS,
//	1, // if you want anti alias at a slight fps cost, it won't make a huge difference on a 2D system
//	EGL_SAMPLES,
//	4,  //keep these 2 lines
	
	// These lines set up some basic internal features that help with depth sorting
	EGL_DEPTH_SIZE,
	8,
	//	These lines tell EGL how the surface is made up.
	EGL_SURFACE_TYPE,
	EGL_WINDOW_BIT,
	EGL_NONE
};	

// we will also need to tell our context what we want.
static const EGLint context_attributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION,
	2,
	EGL_NONE
};


/* We've added a simple routine to create textures
 Create a texture with width and height
*/
GLuint OGL::CreateTexture2D(int width, int height,  char* TheData)
{
	// Texture handle
	GLuint textureId;
	// Set the alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Generate a texture object
	glGenTextures(1, &textureId);
	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, textureId);
	// set it up
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		TheData);
	
	if (glGetError() != GL_NO_ERROR) printf("We got an error"); // its a good idea to test for errors.

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureId;
}

// This will set up the Broadcomm GPU version of an EGL display,

void OGL::init_EGL(Target_State *state, int width, int height)
{
	
		
	std::cout << "init openGLES started\n";

	
// first set up some local variables we will need to work with	
	
	EGLBoolean result;
	EGLint num_config;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	EGLConfig config;
	
// now lets do the work and set things up
	memset(state, 0, sizeof(Target_State)); // we need to make sure the state is clear

// get an EGL display connection
	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

// initialize the EGL display connection
	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result); // just in case, we should test it.
	
// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

// get an appropriate EGL frame buffer configuration
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);


// create an EGL rendering context
	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
	assert(state->context != EGL_NO_CONTEXT);

// create an EGL window surface
//	success = graphics_get_display_size(0 /* LCD */, &state->width, &state->height);
//	assert(success >= 0);

// ok now that the EGL is all set up, lets give it some important info so that our display manager can make a window
	
	state->width = width;
	state->height = height;

	dst_rect.x = 0; // gives us a slight offset away from the top left corner which ak
	dst_rect.y = 0;
	dst_rect.width = width;
	dst_rect.height = height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = width << 16;
	src_rect.height = height << 16;        
	
// now tell the hardware to make the display open to us
	dispman_display = vc_dispmanx_display_open(0 /* LCD */);
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = 	vc_dispmanx_element_add(dispman_update,
		dispman_display,
		0/*layer*/,
		&dst_rect,
		0/*src*/,
		&src_rect,
		DISPMANX_PROTECTION_NONE, 
		0 /*alpha*/,
		0/*clamp*/,
		(DISPMANX_TRANSFORM_T) 0/*transform*/);
// tell our Target_stat structure what we have
	
	state->nativewindow.element = dispman_element;
	state->nativewindow.width = state->width;
	state->nativewindow.height = state->height;
	vc_dispmanx_update_submit_sync(dispman_update);
// now that the display is ready, we have to have a surgace our OpenGLES2.0 systems will draw to, 
	
	state->surface = eglCreateWindowSurface(state->display, config, &(state->nativewindow), NULL);
	assert(state->surface != EGL_NO_SURFACE); // assert is a way to make sure things are working as we expect
// connect the context to the surface
	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result); // if the assertion fails the program will stop here, the nice thing is this only appears in debug mode
	
// we are using transparent images so we need a blend system to stop the transparency showing through to the background screen
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glActiveTexture(GL_TEXTURE0); // we are only going to use 1 active texture for now
	
// if we got through all that ok, we have a display screen ready for us	
	std::cout << "init openGLES finished\n";

	
	// Some OpenGLES2.0 states that we might need
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(TRUE);
	glDepthRangef(0.0f, 1.0f);
	glClearDepthf(1.0f);
	glCullFace(GL_BACK);
}



/*
 This is a fairly standard Shader loader and generator
 enter with the type of shader needed and a pointer to where the code text is
*/
GLuint OGL::LoadShader(GLenum type, const char *shaderSrc)
{	
// see if we can allcoate a new shader
	GLuint TheShader = glCreateShader(type);
	if (TheShader == 0) return FALSE; // return FALSE (0) to indicate we failed to allocate this shader.
// note...allocating a shader is not the same as generating one. It just means a space was made available
// and our next step is to put the code in there.

	// pass the source
	glShaderSource(TheShader, 1, &shaderSrc, NULL);
	
	// Compile the shader
	glCompileShader(TheShader);

	GLint  IsCompiled; // we need a value to be set if we fail or succeed.
	
// Compiling can result in errors, so we need to make sure we find out if it compiled, 
// but also its useful to report the error
	glGetShaderiv(TheShader, GL_COMPILE_STATUS, &IsCompiled);  // if compile works, TheShader will have a non 0 value 

// a compiler fail means we need to fix the error manually, so output the compiler errors to help us	
	if (!IsCompiled)
	{
		GLint RetinfoLen = 0; 
		glGetShaderiv(TheShader, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{ // standard output for errors
			char* infoLog = (char*) malloc(sizeof(char) * RetinfoLen);
			glGetShaderInfoLog(TheShader, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error compiling this shader:\n%s\n", infoLog); // this is a useful formatting print system
			free(infoLog);
		}
		glDeleteShader(TheShader); // we did allocate a shader, but since we failed to compile we need to remove it.
		return FALSE; // indicate our failure to make the shader
	}
	return TheShader; // if we succeeded we will have a valuer in TheShader, return it.
}

///
// Initialize open GL and create our shaders
//
int OGL::Init()
{
// once the BCM has been initialsed and EGL/DISPMANX is up and runnng, all we really need now is some shaders to work with
// since OpenGLES can't actually draw without them, so we will set up a very simple default pair of shaders, and turn them
// into a Program Object which we will set as the default
	

	bcm_host_init(); // make sure we have set up our Broadcom
	
	uint32_t	 scr_width, scr_height;
	graphics_get_display_size(0,&scr_width,	&scr_height);
	
	
	init_EGL(&state, 1024,800); // this will set up the EGL stuff, its complex so put in a seperate method


// we'll need a base colour to clear the screen this sets it up here (we can change it anytime).
// so we'll go for a magenta colour so we can see it
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	
	// this is our shader code, contained in an array of text
	// there are much better ways to do this, we will cover later
	// This v3 shader code handles textures, but also accepts
	// screen size info to create clipspace and scale. 
		
		GLbyte vShaderStr[] =
			"precision mediump float;\n"
	// these are the attributes we have in our GPU or CPU Array, sent here to process		
			"attribute vec4 a_position;\n" // these are relative positions
			"attribute vec2 a_texCoord;\n"
		
	// these are the uniforms our cpu will supply	
					"uniform vec2 u_Scale;\n"
					"uniform vec2 u_position;\n"  // in pixels
					"uniform vec2 u_Screensize;\n" // half sizes
					"uniform vec2 u_Offsets;\n"
	// these are temp new variables	
					"vec4 newPosition = vec4(0.0);\n"
					"vec4 newAttribPosition = a_position;\n" // we can initialise it with our attribute which is a vec4 too
	// the value we pass to the fragment shader	
					"varying vec2 v_texCoord;\n"
// the main body of the shader code here		
				"void main(){\n"
// do the calculations to create ratio and scale
				"newPosition.x = ((u_position.x - u_Screensize.x) / u_Screensize.x);\n"
		 		"newPosition.y = ((u_position.y - u_Screensize.y) / u_Screensize.y);\n;"
// add a bit to copw with offsets		
	//			"newPosition.x -= u_Offsets.x; \n" //
	//			"newPosition.y += u_Offsets.y; \n" // 
			
				"newAttribPosition.x = (a_position.x * ( (1.0 / u_Screensize.x)*u_Scale.x));\n"
				"newAttribPosition.y = (a_position.y * ( (1.0 / u_Screensize.y)*u_Scale.y));\n"
// make the new gl_position		
				"gl_Position = newAttribPosition + newPosition; \n"
				"v_texCoord=a_texCoord;}\n";
				
	
/**********************************************************************/	
// another vertex shader but this is only used by the FastUpdate 2D systems
// the only change is to use attributes supplied in the VBO for pixel positions
// and not uniforms supplied by the CPU on each tile.
/**********************************************************************/		
	
	GLbyte vShaderStr2[] =
	"precision mediump float;\n"
	// these are the attributes we have in our GPU or CPU Array, sent here to process		
			"attribute vec4 a_position;\n" // these are relative positions
			"attribute vec2 a_texCoord;\n"
			"attribute vec2 u_position;\n"  // we are now using u_position as an attribute, we will keep the name for now
	// these are the uniforms our cpu will supply	
	
					"uniform vec2 u_Scale;\n"
					"uniform vec2 u_Screensize;\n" // half sizes
					"uniform vec2 u_Offsets;\n"
		
	// these are temp new variables	
					"vec4 newPosition = vec4(0.0);\n"
					"vec4 newAttribPosition = a_position;\n" // we can initialise it with our attribute which is a vec4 too
					
		
	// the value we pass to the fragment shader	
					"varying vec2 v_texCoord;\n"
				
// the main body of the shader code here		
				"void main(){\n"
// do the calculations to create ratio and scale
		
				"newPosition.x = ((u_position.x - u_Screensize.x) / u_Screensize.x);\n"
				"newPosition.x -= u_Offsets.x; \n" // frig
				
		
		 		"newPosition.y = ((u_position.y - u_Screensize.y) / u_Screensize.y);\n;"
				"newPosition.y += u_Offsets.y; \n" // frig
		
				"newAttribPosition.x = (a_position.x * ( (1.0 / u_Screensize.x)*u_Scale.x));\n"
				"newAttribPosition.y = (a_position.y * ( (1.0 / u_Screensize.y)*u_Scale.y));\n"
// make the new gl_position		
				"gl_Position = newAttribPosition + newPosition; \n"
				"v_texCoord=a_texCoord;}\n";
								
	
// both vertex shaders can use the same fragment shader

	GLbyte fShaderStr[] =
		"precision mediump float;\n"
		"varying vec2 v_texCoord;\n"
		"uniform sampler2D s_texture;\n"
		"void main()\n"
			"{\n"
				"gl_FragColor = texture2D( s_texture, v_texCoord );\n"
			"}\n";
	
	
// this is a special shader for 3D look how much neater it is, all the main data is in the MVP
// we just multiply a resulting vertex position by MVP and done, 
// though this is blindingly fast we are working on close to 100K poly's so our frame rate might drop
	
	GLbyte vShaderStr3D[] =
		"precision mediump float;	\n"
		"attribute vec4 a_position;	\n" // we are really only sending a vec3 but GLSL will pad it out
		"attribute vec4 a_screenPosition;\n" // same here for a vec2
		"attribute vec2 a_texCoord;	\n"
		"uniform mat4 MVP;			\n" // << this is the cool bit, we are going to pass a matrix
		"varying vec2 v_texCoord;	\n"
		
		"void main()				\n"
			"{ \n "  
			" gl_Position =  MVP * (a_position + a_screenPosition);\n" // we allowed it to pad it out so we could add 2 vec4 values (no real worry about the .w componant here)
			" v_texCoord = a_texCoord;  \n" 
			"} \n";

//this is a vertex shader for our sprites,it assumes all the translations are held in the MVP meaning it only has to do a simple MVP* position to work
	GLbyte vShaderStr3DSprite[] =
	"precision mediump float;	\n"
	"attribute vec4 a_position;	\n" // we are really only sending a vec3 but GLSL will pad it out
	"attribute vec2 a_texCoord;	\n"
	"uniform mat4 MVP;			\n" // << this is the cool bit, we are going to pass a matrix
	"varying vec2 v_texCoord;	\n"
		 
	"void main()				\n"
		"{ \n "  
		" gl_Position =  MVP * a_position ;\n" // we allowed it to pad it out so we could add 2 vec4 values (no real worry about the .w componant here)
		" v_texCoord = a_texCoord;  \n"
		"} \n";

	
// shaders next, lets create variables to hold these	
	GLuint vertexShader, fragmentShader; // we need some variables


// Load and compile the vertex/fragment shaders
	vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vShaderStr);
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (char*)fShaderStr);

	if (vertexShader == 0 || fragmentShader == 0) // test if they both have non 0 values
	{
		return FALSE; // one of them is false, the report will have been printed, and we cannot continue
	}
	
// Create the general program object	
	programObject = glCreateProgram();
	
	if (programObject == 0) 	
	{
		cout << "Unable to create our Program Object " << endl;		
		return FALSE; // there was a failure here
	
	}
	
// now we have the V and F shaders  attach them to the progam object
	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	
// Link the program
	glLinkProgram(programObject);
// Check the link status
	GLint AreTheylinked;
	
	glGetProgramiv(programObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
// check and report any errors
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(programObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(programObject);
		return FALSE;
	}
/***************************************************************************/	
// lets repeat the process with a new vertex shader for FastUpdate
/***************************************************************************/	
	vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vShaderStr2);
	// we don't need a new fragment shader
	if(vertexShader == 0) // test it compiled ok (we don't need to test fragment here like last time
	{
		return FALSE; // one of them is false, the report will have been printed, and we cannot continue
	}
	
	// Create a new  program object	for fast updatse
	FastProgramObject = glCreateProgram();
	
	if (FastProgramObject == 0) 	
	{
		cout << "Unable to create our Fast Program Object " << endl;		
		return FALSE; // there was a failure here
	
	}	
/* Time to make a brand new shader pair, for our fastupdate */	
/* simply repeat the process for the 1st Shader pair but now we have a different vertex shader*/	
/* now we have the new V and old F shaders  attach them to the new progam object */
	
	glAttachShader(FastProgramObject, vertexShader);
	glAttachShader(FastProgramObject, fragmentShader);
	
	// Link the program
	glLinkProgram(FastProgramObject);
	
	glGetProgramiv(FastProgramObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
			glGetProgramiv(FastProgramObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(FastProgramObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking Fast Update Program Object:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(FastProgramObject);
		return FALSE;
	}
	
	/***************************************************************************/	
	// lets repeat the process with a new vertex shader for 3D shading
	/***************************************************************************/	
		vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vShaderStr3D);
	// we don't need a new fragment shader
	if(vertexShader == 0) // test it compiled ok (we don't need to test fragment here like last time
	{
		return FALSE; // one of them is false, the report will have been printed, and we cannot continue
	}
	
	// Create a new  program object	for fast updatse
	ProgramObject3D = glCreateProgram();
	
	if (ProgramObject3D  == 0) 	
	{
		cout << "Unable to create our 3D Program Object " << endl;		
		return FALSE; // there was a failure here
	
	}	
	
	glAttachShader(ProgramObject3D, vertexShader);
	glAttachShader(ProgramObject3D, fragmentShader);
	
	// Link the program
	glLinkProgram(ProgramObject3D);
	
	glGetProgramiv(ProgramObject3D, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
			glGetProgramiv(ProgramObject3D, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(ProgramObject3D, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking 3D Program Object:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(ProgramObject3D);
		return FALSE;
	}
// create a new PO for sprites
	
	ProgramObjectSprites = glCreateProgram();
	
	vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vShaderStr3DSprite);
	
	if (ProgramObjectSprites  == 0) 	
	{
		cout << "Unable to create our Program Object for sprites" << endl;		
		return FALSE; // there was a failure here
	
	}	
	
	glAttachShader(ProgramObjectSprites, vertexShader);
	glAttachShader(ProgramObjectSprites, fragmentShader);
	
	// Link the program
	glLinkProgram(ProgramObjectSprites);
	
	glGetProgramiv(ProgramObjectSprites, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
			glGetProgramiv(ProgramObjectSprites, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(ProgramObjectSprites, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking Program Object Sprites:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(ProgramObjectSprites);
		return FALSE;
	}
	
	
// hopefully you will notice this process is identical except for the shaders we feed it
// and the final destination of the resulting program object.....making this a great candidate for a method/function rather 
// repeating code?
	

	// now as a final additon we need a couple of text render shaders, 
	
	GLbyte vTextShaderStr[] =
			
			"attribute vec2 a_position;\n" // these are relative positions
			"attribute vec2 a_texCoord;\n"
			"varying vec2 v_texCoord;\n"
			"uniform mat4 projection;"


			"void main()\n"
			"{\n"
			"	gl_Position = projection * vec4(a_position.xy, 0,1.0);\n"
			"	v_texCoord = a_texCoord;\n"
			"}\n";
	
	GLbyte fTextShaderStr[] =
		
		"varying vec2 v_texCoord;\n"
		"uniform sampler2D s_texture2;\n"
		"vec3 textColor = vec3(1.0,0.0,0.1);\n" // for now we will use a fixed colour, later when we have a bettery maths lib we can pass a vec3 or vec4
		"void main()\n"
			"{\n"
			"vec4 sampled = vec4(1.0, 1.0,  texture2D(s_texture2, v_texCoord).ba);\n" // gets a 0 or non 0 value
		
			"gl_FragColor = vec4(textColor, 1.0)* sampled ;\n"
			"}\n";		
	
	
	
	// Create a new  program object	for fast updates
	TextProgramObject = glCreateProgram();
	
	if (TextProgramObject == 0) 	
	{
		cout << "Unable to create our Text Program Object\n" << endl;		
		return FALSE; // there was a failure here
	
	}	
// exactly as we did before lets make up a new PO....did you notice we are repeating a lot of things?
	// we can reused these
	
	
// Load and compile the vertex/fragment shaders
	vertexShader = LoadShader(GL_VERTEX_SHADER, (char*)vTextShaderStr);
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (char*)fTextShaderStr);

	if (vertexShader == 0 || fragmentShader == 0) // test if they both have non 0 values
		{
			return FALSE; // one of them is false, the report will have been printed, and we cannot continue
		}
	
	glAttachShader(TextProgramObject, vertexShader);
	glAttachShader(TextProgramObject, fragmentShader);
	
	// Link the program
	glLinkProgram(TextProgramObject);
	
	glGetProgramiv(TextProgramObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
			glGetProgramiv(TextProgramObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(TextProgramObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking Text Program Object:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(TextProgramObject);
		return FALSE;
	}
	
	
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(programObject); // we will use differnt Program ojbects but for now set up the original as a default
	return TRUE;
}


// since we don't currenty have a maths lib we need to set up a matrix manually, this is how we set an ortho matrix
// its an odd thing but the values are obvious and when set up it puts our graphics in the view plane as we want them
void OGL::setOrthoMatrix(
	
    const float &bottom,
	const float &top,
	const float &left,
	const float &right, 
	const float &near, // how close do we want to be able to see things in the Z plane
	const float &far  // how far before we cut things off in the Zplane
	) 
{ 
	// set an OpenGL Orthographic projection matrix
	OrthoMatrix[0][0] = 2 / (right - left); 
	OrthoMatrix[0][1] = 0; 
	OrthoMatrix[0][2] = 0; 
	OrthoMatrix[0][3] = 0; 
 
	OrthoMatrix[1][0] = 0; 
	OrthoMatrix[1][1] = 2 / (top - bottom); 
	OrthoMatrix[1][2] = 0; 
	OrthoMatrix[1][3] = 0; 
 
	OrthoMatrix[2][0] = 0; 
	OrthoMatrix[2][1] = 0; 
	OrthoMatrix[2][2] = -2 / (far - near); 
	OrthoMatrix[2][3] = 0; 
 
	OrthoMatrix[3][0] = -(right + left) / (right - left); 
	OrthoMatrix[3][1] = -(top + bottom) / (top - bottom); 
	OrthoMatrix[3][2] = -(far + near) / (far - near); 
	OrthoMatrix[3][3] = 1; 
} 