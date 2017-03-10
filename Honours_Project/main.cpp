#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>

#include <iostream>

#include "window.h"
#include "vr_system.h"
#include "scene.h"
#include "point_cloud.h"

// TODO:
// - Clear each eye to a diffrent colour
//    works when clearing the resolve texture directly
//    desn't seem to work when clearing the render texture and blitting to the resolve
//
// - Render a triangle to each eye
// - get blitting from multisampled from non multisampled texture working


void set_gl_attribs();

int main(int argc, char** argv)
{
	// Setup
	Window* window;
	VRSystem* vr_system;
	Scene scene;
	PointCloud point_cloud;

	bool running = true;
	window = Window::get();
	if( !window ) running = false;
	vr_system = VRSystem::get();
	if( !vr_system ) running = false;

	if( running )
	{
		scene.init();
		point_cloud.init();
	}

	while( running )
	{
		SDL_Event sdl_event;
		while( SDL_PollEvent( &sdl_event ) )
		{
			if( sdl_event.type == SDL_QUIT ) running = false;
			else if( sdl_event.type == SDL_KEYDOWN )
			{
				if( sdl_event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) running = false;
			}
		}

		// THE RENDER TEXTURE IS CLEARED WHEN
		// - render texture is not multisampled
		// - But blitting to the resolve buffer is not working

		vr_system->processVREvents();
		vr_system->manageDevices();
		vr_system->updatePoses();

		vr_system->bindEyeTexture( vr::Eye_Left );
		//glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Left ) );
		//glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );
	
		set_gl_attribs();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		scene.render( vr::Eye_Left );
		point_cloud.render( vr::Eye_Left );
		vr_system->drawControllers( vr::Eye_Left );

		vr_system->bindEyeTexture( vr::Eye_Right );
		//glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Right ) );
		//glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );

		set_gl_attribs();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		scene.render( vr::Eye_Right );
		point_cloud.render( vr::Eye_Right );
		vr_system->drawControllers( vr::Eye_Right );

		vr_system->blitEyeTextures();
		vr_system->submitEyeTextures();
		
		window->render( vr_system->renderEyeTexture( vr::Eye_Left ), vr_system->renderEyeTexture( vr::Eye_Right ) );
		window->present();
	}

	// Cleanup
	scene.shutdown();
	if( vr_system ) delete vr_system;
	if( window ) delete window;

	return 0;
}

void set_gl_attribs()
{
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glClearColor( 0.01f, 0.01f, 0.01f, 1.0f );
	glClearDepth( 1.0f );
}