#include "sphere.h"
#include "shader_program.h"
#include <vector>
#include <gtc/type_ptr.hpp>
#include "vr_system.h"

ShaderProgram* Sphere::shader_ = nullptr;
GLint shader_view_mat_location_ = 0;
GLint shader_proj_mat_location_ = 0;
GLint shader_modl_mat_location_ = 0;

Sphere::Sphere()
{

}

Sphere::~Sphere()
{
	shutdown();
}

void Sphere::init()
{
	glGenVertexArrays( 1, &vao_ );
	glBindVertexArray( vao_ );
	glGenBuffers( 1, &vbo_ );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_ );

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	offset += sizeof( GLfloat ) * 3;
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	shader_modl_mat_location_ = shader_->getUniformLocation( "model" );
	shader_view_mat_location_ = shader_->getUniformLocation( "view" );
	shader_proj_mat_location_ = shader_->getUniformLocation( "projection" );
}

void Sphere::shutdown()
{
	if( vao_ ) {
		glDeleteVertexArrays( 1, &vao_ );
		vao_ = 0;
	}
	if( vbo_ ) {
		glDeleteBuffers( 1, &vbo_ );
		vbo_ = 0;
	}
}

void Sphere::update( float dt )
{
	glBindVertexArray( vao_ );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_ );
	std::vector<GLfloat> verts;

	const float incr = 6.283f / (float)segments_;
	for( int i = 0; i < segments_; i++ )
	{
		// first circle
		verts.push_back( std::sin( incr * i ) * radius_ );
		verts.push_back( std::cos( incr * i ) * radius_ );
		verts.push_back( 0 );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );

		verts.push_back( std::sin( incr * (i + 1) ) * radius_ );
		verts.push_back( std::cos( incr * (i + 1) ) * radius_ );
		verts.push_back( 0 );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );

		// second circle
		verts.push_back( std::sin( incr * i ) * radius_ );
		verts.push_back( 0.0f );
		verts.push_back(  std::cos( incr * i ) * radius_ );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );

		verts.push_back( std::sin( incr * (i + 1) ) * radius_ );
		verts.push_back( 0.0f );
		verts.push_back( std::cos( incr * (i + 1) ) * radius_ );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );

		// third circle
		verts.push_back( 0.0f );
		verts.push_back( std::sin( incr * i ) * radius_ );
		verts.push_back( std::cos( incr * i ) * radius_ );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );

		verts.push_back( 0.0f );
		verts.push_back( std::sin( incr * (i + 1) ) * radius_ );
		verts.push_back( std::cos( incr * (i + 1) ) * radius_ );
		verts.push_back( colour_.r ); verts.push_back( colour_.g ); verts.push_back( colour_.b );
	}

	glBufferData( GL_ARRAY_BUFFER, sizeof( verts[0] ) * verts.size(), verts.data(), GL_STREAM_DRAW );
	num_verts_ = verts.size() / 6;
}

void Sphere::render( vr::EVREye	eye )
{
	shader_->bind();
	glUniformMatrix4fv( shader_view_mat_location_, 1, GL_FALSE, glm::value_ptr( VRSystem::get()->viewMatrix( eye ) ) );
	glUniformMatrix4fv( shader_proj_mat_location_, 1, GL_FALSE, glm::value_ptr( VRSystem::get()->projectionMartix( eye ) ) );
	glUniformMatrix4fv( shader_modl_mat_location_, 1, GL_FALSE, glm::value_ptr( glm::mat4() ) );

	glBindVertexArray( vao_ );
	glDrawArrays( GL_LINES, 0, num_verts_ );
}