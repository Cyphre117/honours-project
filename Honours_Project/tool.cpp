#include "tool.h"

VRTool::VRTool( VRToolType type ) :
	vr_system_(nullptr),
	controller_(nullptr),
	type_(type),
	initialised_(false),
	active_(false)
{}

VRTool::~VRTool()
{}