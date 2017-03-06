#include "controller.h"
#include "helpers.h"
#include "vr_system.h"
#include <iostream>

Controller::Controller() :
	initialised_( false ),
	vr_system_( nullptr ),
	shader_( nullptr ),
	vao_( 0 ),
	model_mat_location_( 0 ),
	index_( vr::k_unTrackedDeviceIndexInvalid )
{
}

Controller::~Controller()
{

}

void Controller::init( vr::TrackedDeviceIndex_t index, vr::IVRSystem* vr_system, const ShaderProgram& shader )
{
	vr_system_ = vr_system;
	index_ = index;

	initialised_ = true;
	std::cout << "controller " << index << " initialised!" << std::endl;

	GLfloat verts[] = {
		0, 0, 0, 1, 0, 0,
		.2, 0, 0, 1, 0, 0,

		0, 0, 0, 0, 1, 0,
		0, .2, 0, 0, 1, 0,

		0, 0, 0, 0, 0, 1,
		0, 0, .2, 0, 0, 1,
	};

	shader.bind();
	glGenVertexArrays( 1, &vao_ );
	glBindVertexArray( vao_ );

	GLuint vbo;
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( verts ), verts, GL_STATIC_DRAW );

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	GLint posAttrib = shader.getAttribLocation( "vPosition" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offset );

	offset += sizeof( GLfloat ) * 3;
	GLint colAttrib = shader.getAttribLocation( "vColour" );
	glEnableVertexAttribArray( colAttrib );
	glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offset );

	glBindVertexArray( 0 );
	glUseProgram( 0 );
}

void Controller::update()
{
	if( initialised_ )
	{
		// Copy the current state into the old state
		prev_state_ = state_;

		// Update the current controller state and pose
		vr_system_->GetControllerState( index_, &state_, sizeof( state_ ) );

		/*
		// The pos given from this function is accurate at the time the state was generated, i think...
		// It definately feels laggier than the one passed from WaitGetPoses()
		hmd_->GetControllerStateWithPose(
		vr::TrackingUniverseOrigin::TrackingUniverseStanding,
		index_,
		&state_, sizeof( state_ ),
		&pose_ );
		*/
	}
}

void Controller::draw()
{
	glBindVertexArray( vao_ );
	glDrawArrays( GL_LINES, 0, 6 );
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

glm::mat4 Controller::deviceToAbsoluteTracking() const
{
	return convertHMDmat3ToGLMMat4( pose_.mDeviceToAbsoluteTracking );
}
