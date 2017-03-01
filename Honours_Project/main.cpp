#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>

#include <iostream>

#include "window.h"
#include "vr_system.h"

// TODO:
// - Clear each eye to a diffrent colour
//  - works when clearing the resolve texture directly, desn't seem to work when clearing render and blitting
// - Render a triangle to each eye

int main(int argc, char** argv)
{
	// Setup
	Window* window = Window::get();
	VRSystem* vr_system = VRSystem::get();
	ShaderProgram test_shader;

	test_shader.loadFragmentSourceFile( "window_shader_fs.glsl" );
	test_shader.loadVertexSourceFile( "window_shader_vs.glsl" );
	test_shader.init();

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
		vr_system->updatePoses();

		glEnable( GL_DEPTH );
		glClearDepth( 0.0f );

		//vr_system->bindEyeTexture( vr::Eye_Left );
		glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Left ) );

		glClearColor( 1, 0, 0, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//vr_system->bindEyeTexture( vr::Eye_Right );
		glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Right ) );

		glClearColor( 0, 0, 1, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		//glClear( GL_COLOR_BUFFER_BIT );

		vr_system->blitEyeTextures();
		vr_system->submitEyeTextures();
		
		window->render( vr_system->resolveEyeTexture(vr::Eye_Left), vr_system->resolveEyeTexture( vr::Eye_Right ) );
		window->present();
	}

	// Cleanup
	if (vr_system) delete vr_system;
	if (window) delete window;

	return 0;
}