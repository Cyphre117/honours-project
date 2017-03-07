#pragma once
#include "shader_program.h"
#include <glm.hpp>
#include <openvr.h>

class PointCloud
{
public:
	PointCloud();
	~PointCloud();

	bool init();
	void render( vr::EVREye eye );

protected:

	ShaderProgram shader_;
	GLint modl_matrix_location_;
	GLint view_matrix_location_;
	GLint proj_matrix_location_;
	glm::mat4 model_mat_;
	glm::mat4 view_mat_;
	glm::mat4 projection_mat_;

	GLuint vao_;
	GLuint vbo_;
	size_t num_verts_;
};