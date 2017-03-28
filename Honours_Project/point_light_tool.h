#pragma once

#include "tool.h"
#include "glm.hpp"

class ShaderProgram;

class PointLightTool : public VRTool
{
public:
	PointLightTool();
	~PointLightTool();

	bool init() override;
	void shutdown() override;

	void activate() override;
	void deactivate() override;
	void update( float dt ) override;

	// Setters
	void setTargetShader( ShaderProgram** target ) { target_shader_ = target; }
	void setActivateShader( ShaderProgram* activate ) { activate_shader_ = activate; }
	void setDeactivateShader( ShaderProgram* deactivate ) { deactivate_shader_ = deactivate; }

	// Getters
	glm::vec3 lightPos() { return light_pos_; }
	
protected:
	glm::vec3 light_pos_;

	ShaderProgram** target_shader_;
	ShaderProgram* activate_shader_;
	ShaderProgram* deactivate_shader_;
};
