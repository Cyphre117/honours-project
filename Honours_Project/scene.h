#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <openvr.h>
#include <vector>
#include <memory>

#include "shader_program.h"
#include "point_cloud.h"
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

	void init_testing();
	void stop_testing();

	// Hide spheres if testing is disabled
	void toggle_spheres();

	// Getters
	PointCloud* pointCloud() { return &point_cloud_; }

protected:
	Window* window_                    = nullptr;
	VRSystem* vr_system_               = nullptr;
	PointCloud point_cloud_;

	glm::vec3 default_sphere_colour_   = { 1.0f, 0.0f, 1.0f };
	glm::vec3 highlight_sphere_colour_ = { 1.0f, 1.0f, 1.0f };
	std::vector<std::unique_ptr<Sphere>> spheres_;

	void addSphere( glm::vec3 position );

	ShaderProgram shader_;
	GLint modl_matrix_location_ = 0;
	GLint view_matrix_location_ = 0;
	GLint proj_matrix_location_ = 0;
	glm::mat4 model_mat_;

	void start_timer();
	bool test_mode_ = false;
	Uint32 start_time_ = 0;
	Uint32 end_time_ = 0;
	Uint32 previous_time_ = 0;
	std::vector<Uint32> times_;

	void init_floor();
	void render_floor( glm::mat4 view, glm::mat4 projection );

	void init_bunny();
	void init_dragon();

	GLuint floor_vao_        = 0;
	GLsizei num_floor_verts_ = 0;
};