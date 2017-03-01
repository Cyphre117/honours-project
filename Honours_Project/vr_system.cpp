#include "vr_system.h"
#include <SDL.h>
#include <iostream>

// Static member delcarations
VRSystem* VRSystem::self_ = nullptr;

// Constructor
VRSystem::VRSystem() :
	vr_system_(nullptr),
	render_target_width_(0),
	render_target_height_(0),
	near_clip_plane_(0.1f),
	far_clip_plane_(100.0f)
{
}

// Destructor
VRSystem::~VRSystem()
{
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
	
	vr_system_->GetRecommendedRenderTargetSize( &render_target_width_, &render_target_height_ );

	/* SETUP FRAME BUFFERS */
	for( int i = 0; i < 2; i++ )
	{
		// Create render frame buffer
		glGenFramebuffers( 1, &EyeBuffers[i].render_frame_buffer );
		glBindFramebuffer( GL_FRAMEBUFFER, EyeBuffers[i].render_frame_buffer );
		// Attach depth component
		glGenRenderbuffers( 1, &EyeBuffers[i].render_depth );
		glBindRenderbuffer( GL_RENDERBUFFER, EyeBuffers[i].render_depth );
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, render_target_width_, render_target_height_ );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, EyeBuffers[i].render_depth );
		// Attach colour component
		glGenTextures( 1, &EyeBuffers[i].render_texture );
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, EyeBuffers[i].render_texture );
		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, render_target_width_, render_target_height_, true );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, EyeBuffers[i].render_texture, 0 );

		// Create resolve frame buffer
		glGenFramebuffers( 1, &EyeBuffers[i].resolve_frame_buffer );
		glBindFramebuffer( GL_FRAMEBUFFER, EyeBuffers[i].resolve_frame_buffer );
		// Attach colour component
		glGenTextures( 1, &EyeBuffers[i].resolve_texture );
		glBindTexture( GL_TEXTURE_2D, EyeBuffers[i].resolve_texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, render_target_width_, render_target_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EyeBuffers[i].resolve_texture, 0 );

		// Check everything went OK
		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )	{
			std::cout << "Error creating resolve frame buffer!\n" << std::endl;
			success = false;
		}

		// Unbind any bound frame buffer
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		success = true;
	}

	return success;
}

void VRSystem::processVREvents()
{
	vr::VREvent_t event;

	while( vr_system_->PollNextEvent( &event, sizeof( event ) ) )
	{
		// TODO: handle events
	}
}

void VRSystem::bindEyeTexture( vr::EVREye eye )
{
	int index = (eye == vr::Eye_Left ? 0 : 1);

	glEnable( GL_MULTISAMPLE );
	glBindFramebuffer( GL_FRAMEBUFFER, EyeBuffers[index].render_frame_buffer );
	glViewport( 0, 0, render_target_width_, render_target_height_ );
}

void VRSystem::resolveEyeTextures()
{
	for( int i = 0; i < 2; i++ )
	{
		// Blit from the render frame buffer to the resolve
		glBindFramebuffer( GL_READ_BUFFER, EyeBuffers[i].render_frame_buffer );
		glBindFramebuffer( GL_DRAW_BUFFER, EyeBuffers[i].resolve_frame_buffer );
		glBlitFramebuffer(
			0, 0, render_target_width_, render_target_height_,
			0, 0, render_target_width_, render_target_height_,
			GL_COLOR_BUFFER_BIT, GL_LINEAR );
	}
}

void VRSystem::submitEyeTextures()
{
	// TODO: only submit if we have input focus

	// NOTE: to find out what the error codes mean Ctal+F 'enum EVRCompositorError' in 'openvr.h'
	vr::EVRCompositorError error = vr::VRCompositorError_None;

	vr::Texture_t left = { (void*)EyeBuffers[0].resolve_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	error = vr::VRCompositor()->Submit( vr::Eye_Left, &left, NULL );
	if( error != vr::VRCompositorError_None ) std::cout << "ERROR: left eye " << error << std::endl;

	vr::Texture_t right = { (void*)EyeBuffers[1].resolve_texture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	error = vr::VRCompositor()->Submit( vr::Eye_Right, &right, NULL );
	if( error != vr::VRCompositorError_None ) std::cout << "ERROR: right eye " << error << std::endl;

	// Added on advice from comments in IVRCompositor::submit in openvr.h
	glFlush();
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

glm::mat4 VRSystem::convertHMDmat3ToGLMMat4( vr::HmdMatrix34_t matrix )
{
	return glm::mat4(
		matrix.m[0][0], matrix.m[1][0], matrix.m[2][0], 0.0,
		matrix.m[0][1], matrix.m[1][1], matrix.m[2][1], 0.0,
		matrix.m[0][2], matrix.m[1][2], matrix.m[2][2], 0.0,
		matrix.m[0][3], matrix.m[1][3], matrix.m[2][3], 1.0f
	);
}

glm::mat4 VRSystem::convertHMDmat4ToGLMmat4( vr::HmdMatrix44_t matrix )
{
	return glm::mat4(
		matrix.m[0][0], matrix.m[1][0], matrix.m[2][0], matrix.m[3][0],
		matrix.m[0][1], matrix.m[1][1], matrix.m[2][1], matrix.m[3][1],
		matrix.m[0][2], matrix.m[1][2], matrix.m[2][2], matrix.m[3][2],
		matrix.m[0][3], matrix.m[1][3], matrix.m[2][3], matrix.m[3][3]
	);
}