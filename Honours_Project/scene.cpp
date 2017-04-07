#include "scene.h"
#include "window.h"
#include "vr_system.h"
#include "imgui\imgui.h"

#include <gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <ctime>

Scene::Scene() {}

Scene::~Scene() {}

bool Scene::init()
{
	window_ = Window::get();
	vr_system_ = VRSystem::get();

	shader_.loadVertexSourceFile( "colour_shader_vs.glsl" );
	shader_.loadFragmentSourceFile( "colour_shader_fs.glsl" );
	shader_.init();

	modl_matrix_location_ = shader_.getUniformLocation( "model" );
	view_matrix_location_ = shader_.getUniformLocation( "view" );
	proj_matrix_location_ = shader_.getUniformLocation( "projection" );
	
	// Play sounds during testing to help the user
	//init_audio();

	// Give the user some ground to stand on
	init_floor();

	// Init the point cloud
	point_cloud_.setActiveShader( &shader_ );
	point_cloud_.setMoveTool( vr_system_->moveTool() );
	point_cloud_.init();

	init_bunny();
	//init_testing();
	//stop_testing();

	return true;
}

void Scene::shutdown()
{
	//shutdown_audio();

	glDeleteVertexArrays( 1, &floor_vao_ );
	floor_vao_ = 0;
}

void Scene::update( float dt )
{
	for( auto& s : spheres_ )
	{
		// Parent all spheres to the point cloud
		s->setParentTransform( point_cloud_.combinedOffsetMatrix() );

		// Highlight those touching the cursor
		if( s->isTouching( vr_system_->pointerTool()->sphere() ) )
		{
			s->setColour( highlight_sphere_colour_ );
		} else {
			s->setColour( default_sphere_colour_ );
		}
	}

	if( test_mode_ )
	{
		// Check if the pointer is touching the next sphere in the list
		if( spheres_[sphere_indecies_.back()]->isTouching( vr_system_->pointerTool()->sphere() ) )
		{
			// If it's the first sphere, we can start the timer
			if( spheres_.size() == sphere_indecies_.size() )
			{
				start_timer();
			}

			// Print out which sphere was hit
			Uint32 current_time = SDL_GetTicks();
			std::cout << "\t: " << sphere_indecies_.back() << " took "
				<< (current_time - previous_time_) / 1000.0f << " (s)" << std::endl;
			previous_time_ = current_time;

			// Deactivate the sphere that was hit
			spheres_[sphere_indecies_.back()]->setActive( false );
			sphere_indecies_.pop_back();

			// Activate the next sphere
			if( sphere_indecies_.empty() )
			{
				stop_testing();
			}
			else
			{
				spheres_[sphere_indecies_.back()]->setActive( true );
			}
		}
	}

	/*
	for( int i = 0; i < spheres_.size(); i++ )
	{
		// Highlight spheres that are touching the pointer
		if( spheres_[i]->isTouching( vr_system_->pointerTool()->sphere() ) )
		{
			spheres_[i]->setColour( highlight_sphere_colour_ );

			if( test_mode_ )
			{
				if( spheres_.size() == sphere_indecies_.size() )
				{
					// Start the timer when the user touches the first sphere
					start_timer();
				}
				else
				{
					Uint32 current_time = SDL_GetTicks();
					std::cout << "\t" << i - 1 << " -> " << i << " : "
						<< (current_time - previous_time_) / 1000.0f << " (s)" << std::endl;
					previous_time_ = current_time;
				}

				spheres_[i]->setActive( false );
				int next = i + 1;
				if( next >= spheres_.size() )
				{
					next = 0;
					stop_testing();
				}

				spheres_[next]->setActive( true );
			}
		}
	}
	*/

	// Update all the spheres
	for( auto& s : spheres_ )
	{
		s->update( dt );
	}

	/*
	// Helper tool for positioning spheres
	Controller* right_ctrl = vr_system_->rightControler();
	static int selected_sphere_index = -1;
	static glm::vec3 sphere_pos = glm::vec3();

	if( right_ctrl && right_ctrl->isButtonDown( vr::k_EButton_SteamVR_Trigger ) )
	{
		// Click on a sphere to select it
		if( right_ctrl->isButtonPressed( vr::k_EButton_SteamVR_Trigger ) )
		{
			for( int i = 0; i < spheres_.size(); i++ )
			{
				if( spheres_[i]->isTouching( vr_system_->pointerTool()->sphere() ) )
				{
					selected_sphere_index = i;
				}
			}
		}

		if( selected_sphere_index >= 0 )
		{
			spheres_[selected_sphere_index]->setPosition( spheres_[selected_sphere_index]->position() + right_ctrl->velocity() * 0.02f );
			sphere_pos = spheres_[selected_sphere_index]->position();
		}
	}

	// Move the selected sphere
	if( selected_sphere_index >= 0 )
	{
		spheres_[selected_sphere_index]->setColour( highlight_sphere_colour_ );
		if( right_ctrl && right_ctrl->isButtonPressed( vr::k_EButton_Grip ) )
		{
			std::cout << "Sphere[" << selected_sphere_index << "] at: " << sphere_pos.x << ", " << sphere_pos.y << ", " << sphere_pos.z << std::endl;
		}
	}

	// Display the position
	ImGui::Text( "Target position: %.3f %.3f %.3f", sphere_pos.x, sphere_pos.y, sphere_pos.z );
	*/
}

void Scene::render( glm::mat4 view, glm::mat4 projection )
{
	// Parent all spheres to the point cloud
	for( auto& sphere : spheres_ )
	{
		sphere->render( view, projection );
	}

	render_floor( view, projection );
	point_cloud_.render( view, projection );
}

void Scene::init_floor()
{
	// Create circular floor grid
	{
		std::vector<GLfloat> verts;

		int rings = 10;
		float max_radius = 10.0f;

		int segments = 60;
		float inc = (3.1415f*2.0f) / (float)segments;

		for( int r = 0; r < rings; r++ )
		{
			for( int i = 0; i < segments; i++ )
			{
				verts.push_back( std::sin( i * inc ) * max_radius * (r / float( rings )) );
				verts.push_back( 0.0f );
				verts.push_back( std::cos( i * inc ) * max_radius * (r / float( rings )) );
				verts.push_back( (1.0f - (r / float( rings ))) * 0.0f );
				verts.push_back( (1.0f - (r / float( rings ))) * 0.8f );
				verts.push_back( (1.0f - (r / float( rings ))) * 0.9f );

				if( i == segments - 1 )
				{
					verts.push_back( std::sin( 0.0f ) * max_radius * (r / float( rings )) );
					verts.push_back( 0.0f );
					verts.push_back( std::cos( 0.0f ) * max_radius * (r / float( rings )) );
				}
				else {
					verts.push_back( std::sin( (i + 1) * inc ) * max_radius * (r / float( rings )) );
					verts.push_back( 0.0f );
					verts.push_back( std::cos( (i + 1) * inc ) * max_radius * (r / float( rings )) );
				}

				verts.push_back( (1.0f - (r / float( rings ))) * 0.0f );
				verts.push_back( (1.0f - (r / float( rings ))) * 0.8f );
				verts.push_back( (1.0f - (r / float( rings ))) * 0.9f );
			}
		}

		// create triangle at feet
		verts.push_back( 0.0f ); verts.push_back( 0.0f ); verts.push_back( -0.7f );
		verts.push_back( 0.2f ); verts.push_back( 0.5f ); verts.push_back( 0.2f );

		verts.push_back( 0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back( 0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back( -0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back( -0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back( 0.0f ); verts.push_back( 0.0f ); verts.push_back( -0.7f );
		verts.push_back( 0.2f ); verts.push_back( 0.5f ); verts.push_back( 0.2f );

		GLuint vbo;
		glGenVertexArrays( 1, &floor_vao_ );
		glBindVertexArray( floor_vao_ );
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( verts[0] ) * verts.size(), verts.data(), GL_STATIC_DRAW );
		num_floor_verts_ = (GLsizei)verts.size() / 6;

		GLuint stride = 2 * 3 * sizeof( GLfloat );
		GLuint offset = 0;

		GLint posAttrib = shader_.getAttribLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		offset += sizeof( GLfloat ) * 3;
		GLint colAttrib = shader_.getAttribLocation( "vColour" );
		glEnableVertexAttribArray( colAttrib );
		glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );
	}
}

void Scene::render_floor( glm::mat4 view, glm::mat4 projection )
{
	shader_.bind();

	model_mat_ = glm::mat4( 1.0 );

	// Send matricies
	glUniformMatrix4fv( modl_matrix_location_, 1, GL_FALSE, glm::value_ptr( model_mat_ ) );
	glUniformMatrix4fv( view_matrix_location_, 1, GL_FALSE, glm::value_ptr( view ) );
	glUniformMatrix4fv( proj_matrix_location_, 1, GL_FALSE, glm::value_ptr( projection ) );

	glBindVertexArray( floor_vao_ );
	glDrawArrays( GL_LINES, 0, num_floor_verts_ );
}

void Scene::init_bunny()
{
	point_cloud_.loadFile( "models/bunny_res1.ply" );

	// Place spheres
	spheres_.clear();
	addSphere( { -0.313105,  0.46811,   0.189758 } );
	addSphere( { -0.0528103, 0.703198, -0.0966756 } );
	addSphere( { -0.25695,   0.689998, -0.221692 } );
	addSphere( { -0.108166,  0.483527,  0.0260836 } );
	addSphere( { -0.269904,  0.149032,  0.0173241 } );

	addSphere( { -0.247861,  0.144324,  0.137183 } );
	addSphere( { -0.0895841, 0.361093,  0.186468 } );
	addSphere( {  0.221043,  0.272841,  0.0492226 } );
	addSphere( { -0.0794307, 0.355922, -0.12658 } );
	addSphere( { -0.288804,  0.587767,  0.118993 } );

	for( auto& sphere : spheres_ )
	{
		sphere->setColour( default_sphere_colour_ );
	}
}

void Scene::init_dragon()
{
	point_cloud_.loadFile( "models/dragon_res2.ply" );
}

void Scene::addSphere( glm::vec3 position )
{
	spheres_.push_back( std::unique_ptr<Sphere>( new Sphere( position ) ) );
	spheres_.back()->setParentTransform( point_cloud_.combinedOffsetMatrix() );
	spheres_.back()->setShader( &shader_ );
	spheres_.back()->setRadius( 0.01f );
	spheres_.back()->init();
}

void Scene::init_testing()
{
	if( test_mode_ )
	{
		std::cout << "WARNING: testing was cancelled early!" << std::endl;
		start_time_ = SDL_GetTicks();
		stop_testing();
	}

	// Populate sphere indecies
	sphere_indecies_.clear();
	for( size_t i = 0; i < spheres_.size(); i++ )
	{
		sphere_indecies_.push_back( i );
	}

	// Then randomize the order
	std::random_shuffle( sphere_indecies_.begin(), sphere_indecies_.end() );

	if( !spheres_.empty() )
	{
		// Deactivate all the spheres
		for( auto& s : spheres_ )
		{
			s->setActive( false );
		}

		// Except one
		spheres_[sphere_indecies_.back()]->setActive( true );
	}

	test_mode_ = true;
}

void Scene::start_timer()
{
	times_.clear();
	
	start_time_ = SDL_GetTicks();
	previous_time_ = start_time_;
	
	times_.push_back( start_time_ );

	std::cout << "[ Beginning testing ]" << std::endl;
}

void Scene::stop_testing()
{
	end_time_ = SDL_GetTicks();
	test_mode_ = false;
	times_.push_back( end_time_ );

	Uint32 time_taken = end_time_ - start_time_;

	std::cout << "[ Done testing ]" << std::endl;
	std::cout << "\tTotal time taken : " << time_taken/1000.0f << " (s)" << std::endl;

	for( auto& sphere : spheres_ )
	{
		sphere->setActive( true );
	}

	// Format the current time as the name for the file
	time_t t = std::time( nullptr );
	std::tm tm;
	localtime_s( &tm, &t );

	std::ostringstream oss;
	oss << std::put_time( &tm, "%Y-%m-%d_%H:%M:%S" ) << ".txt";
	std::string filename = oss.str();

	std::ofstream logfile(filename, std::ios::ate );

	// Dump the times in a log file
	logfile << "Started at: " << times_[0] << std::endl;

	for( int i = 1; i < times_.size() - 1; i++ )
	{
		logfile << i << ":\t" << times_[i] << std::endl;
	}

	logfile << "Finished at: " << times_.back() << std::endl;
	logfile << "Time taken: " << (times_.back() - times_.front()) / 1000.0f << " (s)" << std::endl;

	logfile.close();
	std::cout << "Written log: " << filename << std::endl;
}

void Scene::toggle_spheres()
{
	if( !test_mode_ )
	{
		if( !spheres_.empty() && spheres_[0]->active() )
		{
			for( auto& s : spheres_ )
			{
				s->setActive( false );
			}
		}
		else
		{
			for( auto& s : spheres_ )
			{
				s->setActive( true );
			}
		}
	}
}

 void Scene::audio_callback( void* userdata, Uint8* stream, int length )
{
	if( !userdata ) return;

	AudioData* data = (AudioData*)userdata;
	if( data->remaining <= 0 || data->buffer == nullptr ) return;

	length = (length > data->remaining ? data->remaining : length);
	SDL_MixAudio( stream, data->pos, data->remaining, SDL_MIX_MAXVOLUME );

	data->pos += length;
	data->remaining -= length;
}
 
void Scene::init_audio()
{
	// Load the files
	if( SDL_LoadWAV( "audio/sound.wav", &audio_spec_, &sphere_hit_sound_.buffer, &sphere_hit_sound_.length ) == nullptr )
	{
		std::cout << "ERROR: failed sphere_hit_sound_" << std::endl;
	}
	else sphere_hit_sound_.reset();
	if( SDL_LoadWAV( "audio/sound_start.wav", &audio_spec_, &start_sound_.buffer, &start_sound_.length ) == nullptr )
	{
		std::cout << "ERROR: failed start_sound_" << std::endl;
	}
	else start_sound_.reset();
	if( SDL_LoadWAV( "audio/sound_end.wav", &audio_spec_, &done_sound_.buffer, &done_sound_.length ) == nullptr )
	{
		std::cout << "ERROR: failed done_sound_" << std::endl;
	}
	else done_sound_.reset();

	audio_spec_.callback = Scene::audio_callback;

	// Open the audio device
	if( SDL_OpenAudio( &audio_spec_, nullptr ) )
	{
		std::cout << "ERROR: failed to open audio device" << std::endl;
	}

	SDL_PauseAudio( 0 );
	start_sound_.play( audio_spec_ );
}

void Scene::shutdown_audio()
{
	if( sphere_hit_sound_.buffer ) SDL_FreeWAV( sphere_hit_sound_.buffer );
	if( start_sound_.buffer ) SDL_FreeWAV( start_sound_.buffer );
	if( done_sound_.buffer ) SDL_FreeWAV( done_sound_.buffer );

	SDL_CloseAudio();
}