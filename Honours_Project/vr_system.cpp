#include "vr_system.h"
#include "helpers.h"
#include <gtc/type_ptr.hpp>
#include <SDL.h>
#include <iostream>
#include "point_cloud.h"

// Static member delcarations
VRSystem* VRSystem::self_ = nullptr;

// Constructor
VRSystem::VRSystem() :
	vr_system_(nullptr),
	render_target_width_(0),
	render_target_height_(0),
	near_clip_plane_(0.1f),
	far_clip_plane_(100.0f),
	point_cloud_(nullptr)
{
}

// Destructor
VRSystem::~VRSystem()
{
	glDeleteFramebuffers( 1, &eye_buffers_[0].render_frame_buffer );
	glDeleteFramebuffers( 1, &eye_buffers_[0].resolve_frame_buffer );
	glDeleteFramebuffers( 1, &eye_buffers_[1].render_frame_buffer );
	glDeleteFramebuffers( 1, &eye_buffers_[1].resolve_frame_buffer );

	left_controller_.shutdown();
	right_controller_.shutdown();

	vr::VR_Shutdown();
	// TODO: should i manually delete the vr_system ptr?
	vr_system_ = nullptr;

	self_ = nullptr;
}

VRSystem* VRSystem::get()
{
	// Check if the window exists yet
	if( self_ == nullptr )
	{
		// If the window does not yet exist, create it
		self_ = new VRSystem();
		bool success = self_->init();

		// Check everything went OK
		if( success == false )
		{
			delete self_;
		}
	}

	return self_;
}

bool VRSystem::init()
{
	bool success = true;

	/* PRELIMINARY CHECKS */
	{
		bool is_hmd_present = vr::VR_IsHmdPresent();
		std::cout << "Found HMD: " << (is_hmd_present ? "yes" : "no") << std::endl;
		if( !is_hmd_present ) success = false;

		bool is_runtime_installed = vr::VR_IsRuntimeInstalled();
		std::cout << "Found OpenVR runtime: " << (is_runtime_installed ? "yes" : "no") << std::endl;
		if( !is_runtime_installed ) success = false;

		if( !success )
		{
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "error", "Something is missing...", NULL );
			return success;
		}
	}

	/* INIT THE VR SYSTEM */
	vr::EVRInitError error = vr::VRInitError_None;
	vr_system_ = vr::VR_Init( &error, vr::VRApplication_Scene );
	if( error != vr::VRInitError_None )
	{
		vr_system_ = nullptr;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( error ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		success = false;
		return success;
	}

	/* PRINT SYSTEM INFO */
	std::cout << "Tracking System: " << getDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String, NULL ) << std::endl;
	std::cout << "Serial Number: " << getDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String, NULL ) << std::endl;

	vr_system_->GetRecommendedRenderTargetSize( &render_target_width_, &render_target_height_ );
	std::cout << "HMD requested resolution: " << render_target_width_ << " by " << render_target_height_ << std::endl;

	/* SETUP FRAME BUFFERS */
	for( int i = 0; i < 2; i++ )
	{
		// Create render frame buffer
		glGenFramebuffers( 1, &eye_buffers_[i].render_frame_buffer );						// Create a FBO
		glBindFramebuffer( GL_FRAMEBUFFER, eye_buffers_[i].render_frame_buffer );			// Bind the FBO
		// Attach colour component
		glGenTextures( 1, &eye_buffers_[i].render_texture );																				// Generate a colour texture
		//glBindTexture( GL_TEXTURE_2D, eye_buffers_[i].render_texture );																		// Bind the texture
		//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, render_target_width_, render_target_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );			// Create texture data
		//glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eye_buffers_[i].render_texture, 0 );					// Attach the texture to the bound FBO
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, eye_buffers_[i].render_texture );															// Bind the multisampled texture
		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, render_target_width_, render_target_height_, true );				// Create multisampled data
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, eye_buffers_[i].render_texture, 0 );		// Attach the multisampled texture to the bound FBO

		// Attach depth component
		glGenRenderbuffers( 1, &eye_buffers_[i].render_depth );																				// Generate a render buffer
		glBindRenderbuffer( GL_RENDERBUFFER, eye_buffers_[i].render_depth );																// Bind the render buffer
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, render_target_width_, render_target_height_ );			// Enable multisampling
		//glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, render_target_width_, render_target_height_ );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, eye_buffers_[i].render_depth );			// Attach the the render buffer as a depth buffer 

		// Check everything went OK
		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) {
			std::cout << "ERROR: incomplete render frame buffer!" << std::endl;
			success = false;
		}

		// Create resolve frame buffer
		glGenFramebuffers( 1, &eye_buffers_[i].resolve_frame_buffer );
		glBindFramebuffer( GL_FRAMEBUFFER, eye_buffers_[i].resolve_frame_buffer );
		// Attach colour component
		glGenTextures( 1, &eye_buffers_[i].resolve_texture );
		glBindTexture( GL_TEXTURE_2D, eye_buffers_[i].resolve_texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, render_target_width_, render_target_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eye_buffers_[i].resolve_texture, 0 );

		// TODO: this is only needed because i'm rendering directly to the resolve texture
		//       I should fix it so i render to the multisampled the blit to this, then i can remove this depth component
		// Attach depth component
		glGenRenderbuffers( 1, &eye_buffers_[i].render_depth );																				// Generate a render buffer
		glBindRenderbuffer( GL_RENDERBUFFER, eye_buffers_[i].render_depth );																// Bind the render buffer
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, render_target_width_, render_target_height_ );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, eye_buffers_[i].render_depth );			// Attach the the render buffer as a depth buffer 

		// Check everything went OK
		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )	{
			std::cout << "ERROR: incomplete resolve frame buffer!" << std::endl;
			success = false;
		}

		// Unbind any bound frame buffer
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	/* INIT SHADERS */
	{
		controller_shader_.loadVertexSourceFile("render_model_shader_vs.glsl");
		controller_shader_.loadFragmentSourceFile("render_model_shader_fs.glsl");
		if( !controller_shader_.init() )
		{
			success = false;
			return success;
		}
		else
		{
			controller_shader_modl_mat_locaton_ = controller_shader_.getUniformLocation( "model" );
			controller_shader_view_mat_locaton_ = controller_shader_.getUniformLocation( "view" );
			controller_shader_proj_mat_locaton_ = controller_shader_.getUniformLocation( "projection" );
		}
	}

	return success;
}

void VRSystem::processVREvents()
{
	vr::VREvent_t event;

	// Poll the event queue
	while( vr_system_->PollNextEvent( &event, sizeof( event ) ) )
	{
		if( event.eventType == vr::VREvent_TrackedDeviceRoleChanged )
		{
			if( event.trackedDeviceIndex == left_controller_.index() )
			{
				left_controller_.shutdown();
				std::cout << "Left controller changed role" << std::endl;
			}
			else if( event.trackedDeviceIndex == right_controller_.index() )
			{
				right_controller_.shutdown();
				std::cout << "Right controller changed role" << std::endl;
			}
		}
		else if( event.eventType == vr::VREvent_TrackedDeviceDeactivated )
		{
			if( event.trackedDeviceIndex == left_controller_.index() )
			{
				left_controller_.shutdown();
				std::cout << "Left controller disconncted" << std::endl;
			}
			else if( event.trackedDeviceIndex == right_controller_.index() )
			{
				right_controller_.shutdown();
				std::cout << "Right controller disconncted" << std::endl;
			}
		}
	}
}

void VRSystem::manageDevices()
{
	// Init the left controller if it hasn't been initialised already
	if( !left_controller_.isInitialised() )
	{
		vr::TrackedDeviceIndex_t left_index = vr_system_->GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole::TrackedControllerRole_LeftHand );
		if( left_index != -1 )
		{
			left_controller_.init( left_index, this, controller_shader_ );
			left_controller_.setActiveTool( &move_tool_ );
			std::cout << "Left controller initialised!" << std::endl;
		}
	}

	// Init the right controller if it hasn't been initialised already
	if( !right_controller_.isInitialised() )
	{
		vr::TrackedDeviceIndex_t right_index = vr_system_->GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole::TrackedControllerRole_RightHand );
		if( right_index != -1 )
		{
			right_controller_.init( right_index, this, controller_shader_ );
			right_controller_.setActiveTool( &pointer_tool_ );
			//right_controller_.setActiveTool( &point_light_tool_ );
			std::cout << "Right controller initialised!" << std::endl;
		}
	}
}

void VRSystem::updatePoses()
{
	// Update the pose list
	vr::VRCompositor()->WaitGetPoses( poses_, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	for( int device_index = 0; device_index < vr::k_unMaxTrackedDeviceCount; device_index++ )
	{
		if( poses_[device_index].bPoseIsValid )
		{
			// Translate the pose matrix to a glm::mat4 for ease of use
			// The stored matrix is from device to absolute tracking position, we will probably want that inverted
			transforms_[device_index] = glm::inverse( convertHMDmat3ToGLMMat4( poses_[device_index].mDeviceToAbsoluteTracking ) );
		
			// Pass controller poses to the controllers
			if( left_controller_.index() == device_index ) left_controller_.setPose( poses_[device_index] );
			if( right_controller_.index() == device_index ) right_controller_.setPose( poses_[device_index] );
		}
	}
}

void VRSystem::updateDevices( float dt )
{
	if( left_controller_.isInitialised() )
	{
		left_controller_.update( dt );

		if( left_controller_.isButtonPressed( vr::k_EButton_Grip ) )
		{
			std::cout << "Reset point cloud location" << std::endl;
			point_cloud_->resetPosition();
			move_tool_.resetTransform();
		}
	}

	if( right_controller_.isInitialised() )
	{
		right_controller_.update( dt );

		// While the pointer tool is not 'active' it is always accissible
		// This reveals a desing flaw in the way my controllers + tools were designed but i'll ignore it for now...
		pointer_tool_.update( dt );
	}
}

void VRSystem::render( const glm::mat4& view, const glm::mat4& projection )
{
	drawControllers( view, projection );

	if( move_tool_.isInitialised() ) move_tool_.render( view, projection );
	if( pointer_tool_.isInitialised() ) pointer_tool_.render( view, projection );
	if( point_light_tool_.isInitialised() ) point_light_tool_.render( view, projection );
}

void VRSystem::drawControllers( glm::mat4 view, glm::mat4 projection )
{
	controller_shader_.bind();
	glUniformMatrix4fv( controller_shader_view_mat_locaton_, 1, GL_FALSE, glm::value_ptr( view ) );
	glUniformMatrix4fv( controller_shader_proj_mat_locaton_, 1, GL_FALSE, glm::value_ptr( projection ) );

	if( left_controller_.isInitialised() )
	{
		glUniformMatrix4fv( controller_shader_modl_mat_locaton_, 1, GL_FALSE, glm::value_ptr( left_controller_.deviceToAbsoluteTracking() ) );
		left_controller_.draw();
	}
	if( right_controller_.isInitialised() )
	{
		glUniformMatrix4fv( controller_shader_modl_mat_locaton_, 1, GL_FALSE, glm::value_ptr( right_controller_.deviceToAbsoluteTracking() ) );
		right_controller_.draw();
	}
}

void VRSystem::bindEyeTexture( vr::EVREye eye )
{
	glEnable( GL_MULTISAMPLE );
	glEnable( GL_DEPTH_TEST );
	// TODO: this should be binding the renderEyeTexture instead
	glBindFramebuffer( GL_FRAMEBUFFER, resolveEyeTexture(eye) );
	glViewport( 0, 0, render_target_width_, render_target_height_ );
}

void VRSystem::blitEyeTextures()
{
	for( int i = 0; i < 2; i++ )
	{
		// Blit from the render frame buffer to the resolve
		glViewport( 0, 0, render_target_width_, render_target_height_ );
		glBindFramebuffer( GL_READ_BUFFER, eye_buffers_[i].render_frame_buffer );
		glBindFramebuffer( GL_DRAW_BUFFER, eye_buffers_[i].resolve_frame_buffer );
		glBlitFramebuffer(
			0, 0, render_target_width_, render_target_height_,
			0, 0, render_target_width_, render_target_height_,
			GL_COLOR_BUFFER_BIT, GL_LINEAR );
	}
}

void VRSystem::submitEyeTextures()
{
	if( hasInputFocus() )
	{
		// NOTE: to find out what the error codes mean Ctal+F 'enum EVRCompositorError' in 'openvr.h'
		vr::EVRCompositorError error = vr::VRCompositorError_None;

		vr::Texture_t left = { (void*)eye_buffers_[0].resolve_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		error = vr::VRCompositor()->Submit( vr::Eye_Left, &left, NULL );
		if( error != vr::VRCompositorError_None ) std::cout << "ERROR: left eye  " << error << std::endl;

		vr::Texture_t right = { (void*)eye_buffers_[1].resolve_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		error = vr::VRCompositor()->Submit( vr::Eye_Right, &right, NULL );
		if( error != vr::VRCompositorError_None ) std::cout << "ERROR: right eye " << error << std::endl;

		// Added on advice from comments in IVRCompositor::submit in openvr.h
		glFlush();
	}
}

glm::mat4 VRSystem::projectionMartix( vr::Hmd_Eye eye )
{
	vr::HmdMatrix44_t matrix = vr_system_->GetProjectionMatrix( eye, near_clip_plane_, far_clip_plane_ );
	return convertHMDmat4ToGLMmat4( matrix );
}

glm::mat4 VRSystem::eyePoseMatrix( vr::Hmd_Eye eye )
{
	vr::HmdMatrix34_t matrix = vr_system_->GetEyeToHeadTransform( eye );
	return glm::inverse( convertHMDmat3ToGLMMat4( matrix ) );
}

std::string VRSystem::getDeviceString(
	vr::TrackedDeviceIndex_t device,
	vr::TrackedDeviceProperty prop,
	vr::TrackedPropertyError* error )
{
	uint32_t buffer_length = vr_system_->GetStringTrackedDeviceProperty( device, prop, NULL, 0, error );
	if( buffer_length == 0 ) return "";

	char* buffer = new char[buffer_length];
	vr_system_->GetStringTrackedDeviceProperty( device, prop, buffer, buffer_length, error );
	std::string result = buffer;
	delete[] buffer;
	return result;
}