#include "move_tool.h"
#include "vr_system.h"

MoveTool::MoveTool() :
	VRTool( VRToolType::Move ),
	translation_(0),
	rotation_(0)
{}

MoveTool::~MoveTool()
{}

bool MoveTool::init()
{
	initialised_ = true;
	bool success = true;

	// put init code here

	return success;
}

void MoveTool::shutdown()
{
	initialised_ = false;
}

void MoveTool::activate()
{
	active_ = true;
}

void MoveTool::deactivate()
{
	active_ = false;
}

void MoveTool::update( float dt )
{
	if( vr_system_->leftControler()->isButtonDown( vr::k_EButton_SteamVR_Trigger ) )
	{
		translation_ += controller_->velocity() * dt;
		rotation_ += controller_->angularVelocity() * dt;
	}
}