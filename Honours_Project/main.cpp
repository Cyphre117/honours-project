#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <openvr.h>

#include <iostream>

#include "window.h"
#include "vr_system.h"

// TODO:
// - Clear each eye to a diffrent colour
//    works when clearing the resolve texture directly
//    desn't seem to work when clearing the render texture and blitting to the resolve
//
// - Render a triangle to each eye
// - get blitting from multisampled from non multisampled texture working

Window* window = nullptr;
VRSystem* vr_system = nullptr;

ShaderProgram test_shader;
GLint modl_matrix_location;
GLint view_matrix_location;
GLint proj_matrix_location;
glm::mat4 model_mat;
glm::mat4 view_mat;
glm::mat4 projection_mat;

GLuint scene_vao = 0;

void init_scene();
void render_scene( vr::EVREye eye );

int main(int argc, char** argv)
{
	// Setup
	bool running = true;
	window = Window::get();
	if( !window ) running = false;
	vr_system = VRSystem::get();
	if( !vr_system ) running = false;

	test_shader.loadVertexSourceFile( "colour_shader_vs.glsl" );
	test_shader.loadFragmentSourceFile( "colour_shader_fs.glsl" );
	test_shader.init();

	modl_matrix_location = test_shader.getUniformLocation( "model" );
	view_matrix_location = test_shader.getUniformLocation( "view" );
	proj_matrix_location = test_shader.getUniformLocation( "projection" );
	glm::mat4 model_mat;
	glm::mat4 view_mat;
	glm::mat4 projection_mat;

	init_scene();

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
		vr_system->updatePoses();

		//vr_system->bindEyeTexture( vr::Eye_Left );
		glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Left ) );
		glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );

		glClearColor( 0, 0, 0, 1 );
		//glClearDepth( 0.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		render_scene( vr::Eye_Left );

		//vr_system->bindEyeTexture( vr::Eye_Right );
		glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Right ) );
		glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );

		glClearColor( 0, 0, 0, 1 );
		//glClearDepth( 0.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		render_scene( vr::Eye_Right );

		vr_system->blitEyeTextures();
		vr_system->submitEyeTextures();
		
		window->render( vr_system->resolveEyeTexture( vr::Eye_Left ), vr_system->resolveEyeTexture( vr::Eye_Right ) );
		window->present();
	}

	// Cleanup
	if (vr_system) delete vr_system;
	if (window) delete window;

	return 0;
}

void init_scene()
{	
	// Setup scene data
	{
		// Some hardcoded Triangles
		float vertices[] = {
			// ground triangle
			0.0f, 0.0f, -0.7f, 0.5f, 1.0f, 0.5f,
			0.5f, 0.0f,  0.5f, 0.8f, 0.5f, 0.8f,
			-0.5f, 0.0f,  0.5f, 0.8f, 0.5f, 0.8f,

			// rear triangle
			0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

			// forward left triangle
			-1.0f, 0.9f, -1.0f, 1.0f, 1.0f, 1.0f,
			-0.8f, 0.0f, -1.3f, 0.0f, 1.0f, 1.0f,
			-1.3f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f,

			// forward right triangle
			1.0f, 0.9f, -1.0f, 1.0f, 1.0f, 1.0f,
			0.8f, 0.0f, -1.3f, 0.0f, 1.0f, 1.0f,
			1.3f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f
		};

		glGenVertexArrays( 1, &scene_vao );
		glBindVertexArray( scene_vao );

		GLuint scene_vbo;
		glGenBuffers( 1, &scene_vbo ); // Generate 1 buffer
		glBindBuffer( GL_ARRAY_BUFFER, scene_vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		GLuint stride = 2 * 3 * sizeof( GLfloat );
		GLuint offset = 0;

		GLint posAttrib = test_shader.getAttribLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		offset += sizeof( GLfloat ) * 3;
		GLint colAttrib = test_shader.getAttribLocation( "vColour" );
		glEnableVertexAttribArray( colAttrib );
		glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		glBindVertexArray( 0 );
	}
}

void render_scene( vr::EVREye eye )
{
	test_shader.bind();
	
	model_mat = view_mat = projection_mat = glm::mat4( 1.0 );

	view_mat = vr_system->eyePoseMatrix( eye ) * vr_system->deviceTransform( vr::k_unTrackedDeviceIndex_Hmd );
	projection_mat = vr_system->projectionMartix( eye );

	// Send matricies
	glUniformMatrix4fv( modl_matrix_location, 1, GL_FALSE, glm::value_ptr( model_mat ) );
	glUniformMatrix4fv( view_matrix_location, 1, GL_FALSE, glm::value_ptr( view_mat ) );
	glUniformMatrix4fv( proj_matrix_location, 1, GL_FALSE, glm::value_ptr( projection_mat ) );
	
	glBindVertexArray( scene_vao );
	glDrawArrays( GL_TRIANGLES, 0, 12 );
}