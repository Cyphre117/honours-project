#include "window.h"

#include <iostream>

// Static member delcarations
Window* Window::self_ = nullptr;

// Constructor
Window::Window() :
	win_(nullptr),
	gl_context_(nullptr),
	width_(1920),
	height_(1080)
{
}

// Destructor
Window::~Window()
{
	// Destory objects
	SDL_DestroyWindow( win_ );
	
	// Cleanup pointers
	win_ = nullptr;
	gl_context_ = nullptr;
	self_ = nullptr;

	// Quit libraries
	SDL_Quit();
}

Window* Window::get()
{
	// Check if the window exists yet
	if( self_ == nullptr )
	{
		// If the window does not yet exist, create it
		self_ = new Window();
		bool success = self_->init();

		// Check everything went OK
		if( success == false )
		{
			delete self_;
		}
	}

	return self_;
}

bool Window::init()
{
	bool success = true;

	/* INIT SDL */

	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		std::cout << "ERROR: Could not init SDL!" << std::endl;
		success = false;
	}	

	win_ = SDL_CreateWindow(
		"Thomas Hope - Honours",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width_,
		height_,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);
	if( !win_ ) {
		std::cout << "ERROR: could not create window!" << std::endl;
		success = false;
	}

	/* INIT GL */

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

	gl_context_ = SDL_GL_CreateContext( win_ );
	if( !gl_context_ ) {
		std::cout << "ERROR: could not create GL context!" << std::endl;
		success = false;
	}

	// Disable VSync
	SDL_GL_SetSwapInterval( 0 );
	int vsync = SDL_GL_GetSwapInterval();
	if( vsync != 0 ) {
		std::cout << "WARNING: vsync is enabled, the will likely throttle the Vive framerate!" << std::endl;
	}

	/* INIT GLEW */
	glewExperimental = GL_TRUE;
	if( glewInit() != GLEW_OK ) {
		success = false;
	}

	/* INIT WINDOW SHADER */
	window_shader_.loadVertexSourceFile("window_shader_vs.glsl");
	window_shader_.loadFragmentSourceFile("window_shader_fs.glsl");
	bool result = window_shader_.init();
	if( !result ) {
		std::cout << "ERROR: failed to init window shader!" << std::endl;
	}

	/* CREATE WINDOW MESHS */
	{
		GLfloat verts[] =
		{
			// Left side
			-1.0, -1.0f, 0,		0.0, 0.0,
			0.0, -1.0, 0,		1.0, 0.0,
			-1.0, 1.0, 0,		0.0, 1.0,
			0.0, 1.0, 0,		1.0, 1.0,

			// Right side
			0.0, -1.0, 0,		0.0, 0.0,
			1.0, -1.0, 0,		1.0, 0.0,
			0.0, 1.0, 0,		0.0, 1.0,
			1.0, 1.0, 0,		1.0, 1.0
		};
		GLushort indices[] = { 0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6 };
		glGenVertexArrays( 1, &vao_ );
		glBindVertexArray( vao_ );
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vao_ );
		glBufferData( GL_ARRAY_BUFFER, sizeof( verts ), verts, GL_STATIC_DRAW );
		GLuint ebo;
		glGenBuffers( 1, &ebo );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );
		GLint posAttrib = window_shader_.getAttribLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), 0 );
		GLint uvAttrib = window_shader_.getAttribLocation( "vUV" );
		glEnableVertexAttribArray( uvAttrib );
		glVertexAttribPointer( uvAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), (void*)(3 * sizeof( GLfloat )) );
	}

	return success;
}

void Window::render( GLuint left_eye_texture, GLuint right_eye_texture )
{
	glDisable( GL_DEPTH_TEST );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, width_, height_ );
	glClearColor( 0.0f, 0.5f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	window_shader_.bind();
	glBindVertexArray( vao_ );

	glBindTexture( GL_TEXTURE_2D, left_eye_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );

	glBindTexture( GL_TEXTURE_2D, right_eye_texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)(12) );
}