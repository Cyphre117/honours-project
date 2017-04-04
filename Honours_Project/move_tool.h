#pragma once

#include "tool.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

class MoveTool : public VRTool
{
public:
	MoveTool();
	~MoveTool();

	bool init() override;
	void shutdown() override;

	void activate() override;
	void deactivate() override;
	void update( float dt ) override;
	void render( vr::EVREye eye ) override {}

	// Getters
	glm::mat4 translationMatrix() const { return glm::translate( glm::mat4(), translation_ ); }
	glm::mat4 rotationMatrix() const { return rotation_ == glm::vec3(0) ? glm::mat4() : glm::rotate( glm::mat4(), glm::length( rotation_ ), rotation_ ); }
	glm::vec3 translation() const { return translation_; }
	glm::vec3 rotation() const { return rotation_; }

	// Setters
	void resetTransform() { translation_ = glm::vec3(); rotation_ = glm::vec3(); }

protected:
	glm::vec3 translation_;
	glm::vec3 rotation_;
};