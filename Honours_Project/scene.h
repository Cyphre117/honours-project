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
	
	// Testing
	void start_timer();

	bool test_mode_       = false;
	Uint32 start_time_    = 0;
	Uint32 end_time_      = 0;
	Uint32 previous_time_ = 0;
	std::vector<Uint32> times_;
	std::vector<size_t> sphere_indecies_;

	// Scene
	void init_floor();
	void render_floor( glm::mat4 view, glm::mat4 projection );

	void init_bunny();
	void init_dragon();

	GLuint floor_vao_        = 0;
	GLsizei num_floor_verts_ = 0;

	// Sounds
	void init_audio();
	void shutdown_audio();
	static void audio_callback( void* userdata, Uint8* stream, int length );

	SDL_AudioSpec audio_spec_;

	struct AudioData {
		Uint8* buffer = nullptr;
		Uint32 length = 0;

		Uint8* pos = nullptr;
		Uint32 remaining = 0;

		void reset() {
			pos = buffer;
			remaining = length;
		}
		void play(SDL_AudioSpec& spec) {
			reset();
			spec.userdata = this;
		}
	};

	AudioData start_sound_;       // Played when testing begins
	AudioData done_sound_;        // Played when testing ends
	AudioData sphere_hit_sound_;  // Played when player touches target
	AudioData callback_data_;     // This is the info received by the callback
};