#pragma once
#include "shader_program.h"
#include <glm.hpp>
#include <openvr.h>
#include "ply_loader.h"

class PointCloud
{
public:
	PointCloud();
	~PointCloud();

	bool init();
	void render( vr::EVREye eye );

protected:

	PlyLoader ply_loader_;
	std::vector<GLfloat> data_;

	ShaderProgram shader_;
	GLint modl_matrix_location_;
	GLint view_matrix_location_;
	GLint proj_matrix_location_;
	glm::mat4 model_mat_;
	glm::mat4 view_mat_;
	glm::mat4 projection_mat_;

	GLuint vao_;
	GLuint vbo_;
	GLsizei num_verts_;


	void calculateAABB();

	GLuint aabb_vao_;
	glm::vec3 lower_bound_;
	glm::vec3 upper_bound_;
};