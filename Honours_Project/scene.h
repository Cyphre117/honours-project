#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>
#include "shader_program.h"
#include "sphere.h"

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
	void update( float dt );
	void render( glm::mat4 view, glm::mat4 projection );

protected:
	Window* window_;
	VRSystem* vr_system_;

	ShaderProgram shader_;
	GLint modl_matrix_location_;
	GLint view_matrix_location_;
	GLint proj_matrix_location_;
	glm::mat4 model_mat_;

	void render_floor( glm::mat4 view, glm::mat4 projection );

	GLuint floor_vao_;
	GLsizei num_floor_verts_;

	Sphere sphere_1_;
	Sphere sphere_2_;
};