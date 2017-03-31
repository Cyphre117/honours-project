#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>

#include <gtc/matrix_transform.hpp>
#include <gtx/matrix_decompose.hpp>
#include <iostream>

#include "window.h"
#include "vr_system.h"
#include "scene.h"
#include "point_cloud.h"
#include "imgui/imgui.h"

// TODO:
// - Clear each eye to a diffrent colour
//    works when clearing the resolve texture directly
//    desn't seem to work when clearing the render texture and blitting to the resolve
//
// - get blitting from multisampled to non multisampled texture working
// - hand tool movements are the wrong scale
// - hand tool rotation is not around the controller

void set_gl_attribs();
void draw_gui();

int main(int argc, char** argv)
{
	// Setup
	Window* window;
	VRSystem* vr_system;
	Scene scene;
	PointCloud point_cloud;
	ShaderProgram standard_shader;
	ShaderProgram point_light_shader;

	// First stage initialisation
	bool running = true;
	window = Window::get();
	if( !window ) running = false;
	vr_system = VRSystem::get();
	if( !vr_system ) running = false;
	ImGui::Init( window->SDLWindow() );

	// Second stage initialisation
	if( running )
	{
		// Shaders
		standard_shader.init( "colour_shader_vs.glsl", "colour_shader_fs.glsl" );
		point_light_shader.init( "point_light_shader_vs.glsl", "point_light_shader_fs.glsl" );
		
		// Tools
		vr_system->pointLightTool()->setTargetShader( point_cloud.activeShaderAddr() );
		vr_system->pointLightTool()->setDeactivateShader( &standard_shader );
		vr_system->pointLightTool()->setActivateShader( &point_light_shader );
		vr_system->pointerTool()->setShader( &standard_shader );

		scene.init();

		point_cloud.setActiveShader( &standard_shader );
		point_cloud.init();
	}

	float dt = 0.0;
	Uint32 ticks = SDL_GetTicks();
	Uint32 prev_ticks = ticks;

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

			ImGui::ProcessEvent( &sdl_event );
		}

		// Hand tool tester code
		/*
		if( vr_system->leftControler() )
		{
			if( vr_system->leftControler()->isButtonDown( vr::k_EButton_SteamVR_Trigger ) )
			{
				glm::vec2 delta = vr_system->leftControler()->touchpadDelta();
				glm::mat4 model = point_cloud.modelMatrix();

				glm::vec3 transform_delta = vr_system->leftControler()->velocity() * dt;
				glm::vec3 rotation_delta = vr_system->leftControler()->angularVelocity() * dt;

				model = glm::translate( model, transform_delta );
				model = glm::rotate( model, glm::length(rotation_delta), rotation_delta);
				point_cloud.setModelMatrix( model );
			}
			else if( vr_system->leftControler()->isButtonPressed( vr::k_EButton_Grip ) )
			{
				point_cloud.resetPosition();
			}
		}*/

		point_cloud.setOffsetMatrix( vr_system->moveTool()->originTransform() );

		vr_system->processVREvents();
		vr_system->manageDevices();
		vr_system->updatePoses();
		vr_system->updateDevices( dt );

		ImGui::Frame( window->SDLWindow(), vr_system );

		// THE RENDER TEXTURE IS CLEARED WHEN
		// - render texture is not multisampled
		// - But blitting to the resolve buffer is not working

		vr_system->bindEyeTexture( vr::Eye_Left );
		//glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Left ) );
		//glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );
	
		set_gl_attribs();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		scene.render( vr::Eye_Left );
		point_cloud.render( vr::Eye_Left );
		vr_system->render( vr::Eye_Left );

		draw_gui();
		ImGui::Render();

		vr_system->bindEyeTexture( vr::Eye_Right );
		//glBindFramebuffer( GL_FRAMEBUFFER, vr_system->resolveEyeTexture( vr::Eye_Right ) );
		//glViewport( 0, 0, vr_system->renderTargetWidth(), vr_system->renderTargetHeight() );

		set_gl_attribs();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		scene.render( vr::Eye_Right );
		point_cloud.render( vr::Eye_Right );
		vr_system->render( vr::Eye_Right );

		vr_system->blitEyeTextures();
		vr_system->submitEyeTextures();
		
		window->render( vr_system->renderEyeTexture( vr::Eye_Left ), vr_system->renderEyeTexture( vr::Eye_Right ) );
		window->present();

		// Update dt
		prev_ticks = ticks;
		ticks = SDL_GetTicks();
		dt = (ticks - prev_ticks) / 1000.0f;
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

void draw_gui()
{
	VRSystem* system = VRSystem::get();
	ImGuiIO& IO = ImGui::GetIO();
	IO.MouseDrawCursor = true;

	ImGui::SetWindowFontScale( 3.0f );
	ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
	ImGui::Separator();
	
	Controller* controller = VRSystem::get()->leftControler();
	if( controller )
	{
		ImGui::Text( "Touchpad Delta: (%.2f, %.2f)", controller->touchpadDelta().x, controller->touchpadDelta().y );
		ImGui::Text( "Trigger: %s", controller->isButtonDown( vr::k_EButton_SteamVR_Trigger ) ? "YES" : "NO" );
	}

	ImGui::Text( "Translation: %f %f %f", system->moveTool()->translation().x, system->moveTool()->translation().y, system->moveTool()->translation().z );
	ImGui::Text( "Rotation: %f %f %f", system->moveTool()->rotation().x, system->moveTool()->rotation().y, system->moveTool()->rotation().z );

	ImGui::Text( "Point light position: %f %f %f", system->pointLightTool()->lightPos().x, system->pointLightTool()->lightPos().y, system->pointLightTool()->lightPos().z );
}