#include "point_light_tool.h"
#include "controller.h"
#include "gtx/matrix_decompose.hpp"

PointLightTool::PointLightTool() :
	VRTool( VRToolType::PointLight ),
	target_shader_(nullptr),
	activate_shader_(nullptr),
	deactivate_shader_(nullptr),
	tool_shader_position_location_(0)
{
}

PointLightTool::~PointLightTool()
{
}

bool PointLightTool::init()
{
	initialised_ = true;
	bool success = true;

	// PUT CODE HERE

	return success;
}

void PointLightTool::shutdown()
{
}

void PointLightTool::activate()
{
	active_ = true;
}

void PointLightTool::deactivate()
{
	active_ = false;
}

void PointLightTool::update( float dt )
{
	if( controller_ && controller_->isButtonDown( vr::k_EButton_SteamVR_Trigger ) )
	{
		// Turn on point light rendering
		if( target_shader_ )
		{
			*target_shader_ = activate_shader_;
		}

		// Get the controller position
		glm::mat4 matrix = controller_->deviceToAbsoluteTracking();
		light_pos_ = glm::vec3( matrix[3][0], matrix[3][1], matrix[3][2] );

		activate_shader_->bind();

		// Send the position to the shader
		glUniform3f( tool_shader_position_location_, light_pos_.x, light_pos_.y, light_pos_.x );

		glUseProgram( 0 );
	}
	else
	{
		// Return to normal rendering
		if( target_shader_ )
		{
			*target_shader_ = deactivate_shader_;
		}
	}
}

void PointLightTool::setActivateShader( ShaderProgram* activate )
{
	activate_shader_ = activate;

	tool_shader_position_location_ = activate_shader_->getUniformLocation( "tool_position" );
}