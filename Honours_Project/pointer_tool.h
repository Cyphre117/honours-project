#pragma once

#include "tool.h"
#include "GL/glew.h"
#include "sphere.h"

// TODO:
// - render a line coming out of the controller when the touchpad is pressed
// - change pointer length depending on finder position on the touchpad

class PointerTool : public VRTool
{
public:
	PointerTool();
	~PointerTool();

	bool init() override;
	void shutdown() override;

	void activate() override;
	void deactivate() override;
	void update( float dt ) override;
	void render( const glm::mat4& view, const glm::mat4& projection ) override;

	// Getters

	// If you want to return a sphere copy you will need to define an explicit copy constructor
	const Sphere& sphere() const { return sphere_; }

protected:

	Sphere sphere_;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLsizei num_verts_ = 0;
	GLint shader_view_mat_location_ = 0;
	GLint shader_proj_mat_location_ = 0;
	GLint shader_modl_mat_location_ = 0;
};