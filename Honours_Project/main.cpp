#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>

#include <iostream>

#include "window.h"
#include "vr_system.h"

int main(int argc, char** argv)
{
	// Setup
	Window* window = Window::get();
	VRSystem* vr_system = VRSystem::get();

	bool done = false;
	SDL_Event sdl_event;
	while( !done )
	{
		while( SDL_PollEvent( &sdl_event ) )
		{
			if( sdl_event.type == SDL_QUIT ) done = true;
			else if( sdl_event.type == SDL_KEYDOWN )
			{
				if( sdl_event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) done = true;
			}
		}

		vr_system->processVREvents();

		vr_system->resolveEyeTextures();
		vr_system->submitEyeTextures();
		
		window->render( 0, 0 );
		window->present();
	}

	// Cleanup
	if (vr_system) delete vr_system;
	if (window) delete window;

	return 0;
}