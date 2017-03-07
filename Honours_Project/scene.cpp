#include "scene.h"
#include "window.h"
#include "vr_system.h"
#include <gtc/type_ptr.hpp>

Scene::Scene() :
	window_(nullptr),
	vr_system_(nullptr),
	vao_(0)
{

}

Scene::~Scene()
{

}

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
	
	// Setup scene data
	{
		// Some hardcoded Triangles
		float vertices[] = {
			// ground triangle
			0.0f, 0.0f, -0.7f, 0.5f, 1.0f, 0.5f,
			0.5f, 0.0f,  0.5f, 0.8f, 0.5f, 0.8f,
			-0.5f, 0.0f,  0.5f, 0.8f, 0.5f, 0.8f,

			// rear triangle
			0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

			// forward left triangle
			-1.0f, 0.9f, -1.0f, 1.0f, 1.0f, 1.0f,
			-0.8f, 0.0f, -1.3f, 0.0f, 1.0f, 1.0f,
			-1.3f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f,

			// forward right triangle
			1.0f, 0.9f, -1.0f, 1.0f, 1.0f, 1.0f,
			0.8f, 0.0f, -1.3f, 0.0f, 1.0f, 1.0f,
			1.3f, 0.0f, -0.8f, 1.0f, 1.0f, 1.0f
		};

		glGenVertexArrays( 1, &vao_ );
		glBindVertexArray( vao_ );

		GLuint scene_vbo;
		glGenBuffers( 1, &scene_vbo ); // Generate 1 buffer
		glBindBuffer( GL_ARRAY_BUFFER, scene_vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		GLuint stride = 2 * 3 * sizeof( GLfloat );
		GLuint offset = 0;

		GLint posAttrib = shader_.getAttribLocation( "vPosition" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		offset += sizeof( GLfloat ) * 3;
		GLint colAttrib = shader_.getAttribLocation( "vColour" );
		glEnableVertexAttribArray( colAttrib );
		glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

		glBindVertexArray( 0 );
	}

	return true;
}

void Scene::shutdown()
{
	glDeleteVertexArrays( 1, &vao_ );
	vao_ = 0;
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

	glBindVertexArray( vao_ );
	glDrawArrays( GL_TRIANGLES, 0, 12 );
}