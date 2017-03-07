#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>
#include "shader_program.h"

// Forward declarations
class Window;
class VRSystem;

class Scene
{
public:
	Scene();
	~Scene();

	bool init();
	void shutdown();
	void render( vr::EVREye eye );

protected:
	Window* window_;
	VRSystem* vr_system_;

	ShaderProgram shader_;
	GLint modl_matrix_location_;
	GLint view_matrix_location_;
	GLint proj_matrix_location_;
	glm::mat4 model_mat_;
	glm::mat4 view_mat_;
	glm::mat4 projection_mat_;

	GLuint vao_;
};