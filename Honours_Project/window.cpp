#include "window.h"

#include <iostream>

// Static member delcarations
Window* Window::self_ = nullptr;

// Constructor
Window::Window() :
	win_(nullptr),
	gl_context_(nullptr),
	width_(1280),
	height_(640)
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

	return success;
}