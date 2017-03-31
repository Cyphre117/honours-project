#pragma once
#include "openvr.h"

class VRSystem;
class Controller;
class ShaderProgram;

enum class VRToolType { Unknown, Move, PointLight, Pointer };

class VRTool
{
public:
	~VRTool();

	virtual bool init() = 0;
	virtual void shutdown() = 0;

	virtual void activate() = 0;
	virtual void deactivate() = 0;
	virtual void update( float dt ) = 0;
	virtual void render( vr::EVREye eye ) = 0;

	// Setters
	void setController( Controller* controller ) { controller_ = controller; }
	void setVRSystem( VRSystem* vr_system ) { vr_system_ = vr_system; }
	void setShader( ShaderProgram* shader ) { shader_ = shader; }
	
	// Getters
	VRToolType type() { return type_; }
	ShaderProgram* shader() { return shader_; }
	bool isInitialised() { return initialised_; }
	bool isActive() { return active_; }
	
protected:
	VRTool( VRToolType type );
	
	VRSystem* vr_system_;
	Controller* controller_;
	ShaderProgram* shader_;

	const VRToolType type_;
	bool initialised_;
	bool active_;
};