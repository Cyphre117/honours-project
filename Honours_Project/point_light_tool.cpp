#include "point_light_tool.h"
#include "controller.h"
#include "gtx/matrix_decompose.hpp"

PointLightTool::PointLightTool() :
	VRTool( VRToolType::PointLight ),
	target_shader_(nullptr),
	activate_shader_(nullptr),
	deactivate_shader_(nullptr)
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
	if( target_shader_ )
	{
		*target_shader_ = activate_shader_;
	}
}

void PointLightTool::deactivate()
{
	active_ = false;
	if( target_shader_ )
	{
		*target_shader_ = deactivate_shader_;
	}
}

void PointLightTool::update( float dt )
{
	glm::mat4 matrix = controller_->deviceToAbsoluteTracking();
	light_pos_ = glm::vec3( matrix[3][0], matrix[3][1], matrix[3][2] );
}