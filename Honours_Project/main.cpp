#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>

#include <iostream>

#include "window.h"

// TODO:
// - Draw project diagram

int main(int argc, char** argv)
{
	// Setup
	Window* window = Window::get();

	printf( "lol\n" );
	SDL_Delay( 1000 );	
	printf( "cats!\n" );

	window->present();
	SDL_Delay( 1000 );

	// Cleanup
	delete window;

	return 0;
}