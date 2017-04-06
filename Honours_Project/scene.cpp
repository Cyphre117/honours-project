#include "scene.h"
#include "window.h"
#include "vr_system.h"
#include "imgui\imgui.h"
#include <gtc/type_ptr.hpp>
#include <vector>

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
	
	// Give the user some ground to stand on
	init_floor();

	// Init the point cloud
	point_cloud_.setActiveShader( &shader_ );
	point_cloud_.setMoveTool( vr_system_->moveTool() );
	point_cloud_.init();

	init_bunny();
	start_testing();

	return true;
}

void Scene::shutdown()
{
	glDeleteVertexArrays( 1, &floor_vao_ );
	floor_vao_ = 0;
}

void Scene::update( float dt )
{
	static float time = 0.0f;
	time += dt;

	// Parent all spheres to the point cloud
	for( int i = 0; i < spheres_.size(); i++ )
	{
		spheres_[i]->setParentTransform( point_cloud_.combinedOffsetMatrix() );
		spheres_[i]->setColour( default_sphere_colour_ );

		// Highlight spheres that are touching the pointer
		if( spheres_[i]->isTouching( vr_system_->pointerTool()->sphere() ) )
		{
			spheres_[i]->setColour( highlight_sphere_colour_ );

			if( test_mode_ )
			{
				spheres_[i]->setActive( false );
				int next = i + 1;
				if( next >= spheres_.size() ) next = 0;
				spheres_[next]->setActive( true );
			}
		}
		
		spheres_[i]->update( dt );
	}

	//ImGui::Text( "Spheres touching: %s", sphere_1_.isTouching( sphere_2_ ) ? "yes" : "no" );
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
	addSphere( { 0,0,0 } );
	addSphere( { 0,1,0 } );
	addSphere( { 1,1,0 } );
	addSphere( { 1,1,1 } );

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
	spheres_.back()->setShader( &shader_ );
	spheres_.back()->init();
}

void Scene::start_testing()
{
	test_mode_ = true;
	start_time_ = SDL_GetTicks();

	if( spheres_.size() > 0 )
	{
		spheres_[0]->setActive( true );

		for( int i = 1; i < spheres_.size(); i++ )
		{
			spheres_[i]->setActive( false );
		}
	}
}

void Scene::stop_testing()
{
	test_mode_ = false;
	end_time_ = SDL_GetTicks();

	for( auto& sphere : spheres_ )
	{
		sphere->setActive( true );
	}
}