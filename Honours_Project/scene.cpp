#include "scene.h"
#include "window.h"
#include "vr_system.h"
#include <gtc/type_ptr.hpp>
#include <vector>

Scene::Scene() :
	window_(nullptr),
	vr_system_(nullptr),
	floor_vao_(0)
{
}

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
		
	// Create circular floor grid
	{
		std::vector<GLfloat> verts;

		int rings = 10;
		float max_radius = 10.0f;

		int segments = 60;
		float inc = (3.1415f*2.0f)/(float)segments;

		for( int r = 0; r < rings; r++ )
		{
			for( int i = 0; i < segments; i++ )
			{
				verts.push_back( std::sin( i * inc ) * max_radius * (r / float(rings)) );
				verts.push_back( 0.0f );
				verts.push_back( std::cos( i * inc ) * max_radius * (r / float(rings)) );
				verts.push_back( (1.0f - (r / float(rings))) * 0.0f );
				verts.push_back( (1.0f - (r / float(rings))) * 0.8f );
				verts.push_back( (1.0f - (r / float(rings))) * 0.9f );

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

				verts.push_back( (1.0f - (r / float(rings))) * 0.0f );
				verts.push_back( (1.0f - (r / float(rings))) * 0.8f );
				verts.push_back( (1.0f - (r / float(rings))) * 0.9f );
			}
		}

		// create triangle at feet
		verts.push_back( 0.0f ); verts.push_back( 0.0f ); verts.push_back(-0.7f );
		verts.push_back( 0.2f ); verts.push_back( 0.5f ); verts.push_back( 0.2f );
		
		verts.push_back( 0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back( 0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
		verts.push_back( 0.4f ); verts.push_back( 0.2f ); verts.push_back( 0.4f );

		verts.push_back(-0.5f ); verts.push_back( 0.0f ); verts.push_back( 0.5f );
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

	return true;
}

void Scene::shutdown()
{
	glDeleteVertexArrays( 1, &floor_vao_ );
	floor_vao_ = 0;
}

void Scene::render( vr::EVREye eye )
{
	shader_.bind();

	model_mat_ = view_mat_ = projection_mat_ = glm::mat4( 1.0 );

	view_mat_ = vr_system_->viewMatrix( eye );
	projection_mat_ = vr_system_->projectionMartix( eye );

	// Send matricies
	glUniformMatrix4fv( modl_matrix_location_, 1, GL_FALSE, glm::value_ptr( model_mat_ ) );
	glUniformMatrix4fv( view_matrix_location_, 1, GL_FALSE, glm::value_ptr( view_mat_ ) );
	glUniformMatrix4fv( proj_matrix_location_, 1, GL_FALSE, glm::value_ptr( projection_mat_ ) );

	glBindVertexArray( floor_vao_ );
	glDrawArrays( GL_LINES, 0, num_floor_verts_ );
}