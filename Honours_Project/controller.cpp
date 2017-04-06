#include "controller.h"
#include "helpers.h"
#include "vr_system.h"
#include "tool.h"
#include <iostream>
#include <vector>

Controller::Controller() :
	initialised_( false ),
	vr_system_( nullptr ),
	active_tool_( nullptr ),
	shader_( nullptr ),
	model_mat_location_( 0 ),
	index_( vr::k_unTrackedDeviceIndexInvalid ),
	model_vao_( 0 ),
	model_vbo_( 0 ),
	model_ebo_( 0 ),
	model_texture_( 0 ),
	model_num_verts_( 0 )
{
}

Controller::~Controller()
{
	shutdown();
}

void Controller::init( vr::TrackedDeviceIndex_t index, VRSystem* vr_system, const ShaderProgram& shader )
{
	vr_system_ = vr_system->openVRVRSystem();
	index_ = index;

	initialised_ = true;

	shader.bind();

	// Setup render models
	vr::TrackedPropertyError tracked_property_error;
	std::string render_model_name = vr_system->getDeviceString( index_, vr::Prop_RenderModelName_String, &tracked_property_error );
	vr::EVRRenderModelError render_model_error = vr::VRRenderModelError_Loading;
	std::cout << "Loading render model '" << render_model_name << "'... " << std::flush;

	// Suspend the application while we wait for the render models to load
	while( true )
	{
		render_model_error = vr::VRRenderModels()->LoadRenderModel_Async( render_model_name.c_str(), &vr_model_ );
		if( render_model_error != vr::VRRenderModelError_Loading )
			break;

		SDL_Delay( 10 );
	}

	// Suspend the application while we wait for the texture data
	while( true )
	{
		render_model_error = vr::VRRenderModels()->LoadTexture_Async( vr_model_->diffuseTextureId, &vr_texture_ );
		if( render_model_error != vr::VRRenderModelError_Loading )
			break;
		
		SDL_Delay( 10 );
	}

	if( render_model_error != vr::VRRenderModelError_None )
	{
		std::cout << "FAILED!" << std::endl;
	}
	if( render_model_error == vr::VRRenderModelError_None )
	{
		glGenVertexArrays( 1, &model_vao_ );
		glBindVertexArray( model_vao_ );

		glGenBuffers( 1, &model_ebo_ );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, model_ebo_ );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint16_t ) * vr_model_->unTriangleCount * 3, vr_model_->rIndexData, GL_STATIC_DRAW );
		model_num_verts_ = vr_model_->unTriangleCount * 3;

		glGenTextures( 1, &model_texture_ );
		glBindTexture( GL_TEXTURE_2D, model_texture_ );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, vr_texture_->unWidth, vr_texture_->unHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, vr_texture_->rubTextureMapData );

		glGenBuffers( 1, &model_vbo_ );
		glBindBuffer( GL_ARRAY_BUFFER, model_vbo_ );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vr::RenderModel_Vertex_t ) * vr_model_->unVertexCount, vr_model_->rVertexData, GL_STATIC_DRAW );
		std::cout << "OK " << vr_model_->unTriangleCount << " triangles" << std::endl;

		// Identify the components in the vertex buffer
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vPosition ) );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vNormal ) );
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, rfTextureCoord ) );

		glGenerateMipmap( GL_TEXTURE_2D );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

		GLfloat fLargest;
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest );
	}
	else
	{
		std::cout << "ERROR: could not load render model!" << std::endl;
	}

	glUseProgram( 0 );
}

void Controller::shutdown()
{
	initialised_ = false;
	vr_system_ = nullptr;
	active_tool_ = nullptr;
	ShaderProgram* shader_ = nullptr;
	std::string model_name_ = "";
	model_mat_location_ = 0; // Kinda redundant
	model_num_verts_ = 0;

	if( vr_model_ )
	{
		vr::VRRenderModels()->FreeRenderModel( vr_model_ );
		vr_model_ = nullptr;
	}

	if( vr_texture_ )
	{
		vr::VRRenderModels()->FreeTexture( vr_texture_ );
		vr_texture_ = nullptr;
	}

	if( model_vao_ )
	{
		glDeleteVertexArrays( 1, &model_vao_ );
		model_vao_ = 0;
	}

	if( model_vbo_ )
	{
		glDeleteBuffers( 1, &model_vbo_ );
		model_vbo_ = 0;
	}

	if( model_ebo_ )
	{
		glDeleteBuffers( 1, &model_ebo_ );
		model_ebo_ = 0;
	}

	if( model_texture_ )
	{
		glDeleteTextures( 1, &model_texture_ );
		model_texture_ = 0;
	}
}

void Controller::update( float dt )
{
	if( initialised_ )
	{
		// Copy the current state into the old state
		prev_state_ = state_;

		// Update the current controller state and pose
		vr_system_->GetControllerState( index_, &state_, sizeof( state_ ) );
 
		if( active_tool_ )
		{
			active_tool_->update( dt );
		}
	}
}

void Controller::draw()
{
	glBindTexture( GL_TEXTURE_2D, model_texture_ );
	glBindVertexArray( model_vao_ );
	glDrawElements( GL_TRIANGLES, model_num_verts_, GL_UNSIGNED_SHORT, 0 );
}

void Controller::handleEvent( vr::VREvent_t event )
{
	switch( event.eventType )
	{
	case vr::EVREventType::VREvent_ButtonTouch:
		//printf( "touch button %d\n", event.data.controller.button );
		break;
	case vr::EVREventType::VREvent_ButtonUntouch:
		//printf( "untouch button %d\n", event.data.controller.button );
		break;
	case vr::EVREventType::VREvent_ButtonPress: break;
	case vr::EVREventType::VREvent_ButtonUnpress: break;
	default: break;
	}
}

void Controller::setActiveTool( VRTool* tool )
{
	// deactivate the currently active tool if there is one
	if( active_tool_ )
	{
		active_tool_->deactivate();
	}

	// Switch the active tool
	active_tool_ = tool;

	// Activate the new tool
	if( active_tool_ )
	{
		active_tool_->setVRSystem( VRSystem::get() );
		active_tool_->setController( this );

		// Initialise it if this is the first use
		if( !active_tool_->isInitialised() )
		{
			active_tool_->init();
		}

		active_tool_->activate();
	}
}

glm::vec2 Controller::axis( vr::EVRButtonId button ) const
{
	switch( button )
	{
	case vr::k_EButton_Axis0: return glm::vec2( state_.rAxis[0].x, state_.rAxis[0].y );
	case vr::k_EButton_Axis1: return glm::vec2( state_.rAxis[1].x, state_.rAxis[1].y );
	case vr::k_EButton_Axis2: return glm::vec2( state_.rAxis[2].x, state_.rAxis[2].y );
	case vr::k_EButton_Axis3: return glm::vec2( state_.rAxis[3].x, state_.rAxis[3].y );
	case vr::k_EButton_Axis4: return glm::vec2( state_.rAxis[4].x, state_.rAxis[4].y );
	default:
		std::cout << "ERROR: controller.axis( unknown axis id ) !" << std::endl;
		return glm::vec2();
	}
}

glm::vec2 Controller::prevAxis( vr::EVRButtonId button ) const
{
	switch( button )
	{
	case vr::k_EButton_Axis0: return glm::vec2( prev_state_.rAxis[0].x, prev_state_.rAxis[0].y ); break;
	case vr::k_EButton_Axis1: return glm::vec2( prev_state_.rAxis[1].x, prev_state_.rAxis[1].y ); break;
	case vr::k_EButton_Axis2: return glm::vec2( prev_state_.rAxis[2].x, prev_state_.rAxis[2].y ); break;
	case vr::k_EButton_Axis3: return glm::vec2( prev_state_.rAxis[3].x, prev_state_.rAxis[3].y ); break;
	case vr::k_EButton_Axis4: return glm::vec2( prev_state_.rAxis[4].x, prev_state_.rAxis[4].y ); break;
	default:
		std::cout << "ERROR: controller.prevAxis( unknown axis id ) !" << std::endl;
		return glm::vec2();
	}
}

glm::vec2 Controller::axisDelta( vr::EVRButtonId button ) const
{
	glm::vec2 current = axis( button );
	glm::vec2 prev = prevAxis( button );

	return current - prev;
}

glm::vec2 Controller::touchpadDelta() const
{
	if( axis( vr::k_EButton_SteamVR_Touchpad ) != glm::vec2( 0 ) &&
		prevAxis( vr::k_EButton_SteamVR_Touchpad ) != glm::vec2( 0 ) )
	{
		return axisDelta( vr::k_EButton_SteamVR_Touchpad );
	}
	else
	{
		return glm::vec2( 0 );
	}
}

glm::mat4 Controller::deviceToAbsoluteTracking() const
{
	return convertHMDmat3ToGLMMat4( pose_.mDeviceToAbsoluteTracking );
}
