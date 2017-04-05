#pragma once

#include "shader_program.h"
#include <glm.hpp>
#include <openvr.h>
#include "ply_loader.h"

class MoveTool;

class PointCloud
{
public:
	PointCloud();
	~PointCloud();

	bool init();
	void shutdown();
	void update( float dt );
	void render( const glm::mat4& view, const glm::mat4& projection );
	void resetPosition();
	void loadFile( std::string filepath );

	// Getters
	inline glm::mat4 modelMatrix() { return model_mat_; }
	inline glm::vec3 lowerBound() { return lower_bound_; }
	inline glm::vec3 upperBound() { return upper_bound_; }
	inline ShaderProgram** activeShaderAddr() { return &active_shader_; }

	// Setters
	void setMoveTool( MoveTool* move_tool ) { move_tool_ = move_tool; }
	void setOffsetMatrix( glm::mat4 offset ) { offset_mat_ = offset; }
	void setModelMatrix( const glm::mat4& model ) { model_mat_ = model; }
	void setActiveShader( ShaderProgram* shader ) { active_shader_ = shader; }

protected:

	PlyLoader ply_loader_;
	std::vector<GLfloat> data_;

	ShaderProgram* active_shader_;
	MoveTool* move_tool_;

	GLint modl_matrix_location_;
	GLint view_matrix_location_;
	GLint proj_matrix_location_;
	glm::mat4 offset_mat_;
	glm::mat4 model_mat_;

	GLuint vao_;
	GLuint vbo_;
	GLsizei num_verts_;

	void calculateAABB();

	GLuint aabb_vao_;
	glm::vec3 lower_bound_;
	glm::vec3 upper_bound_;
};