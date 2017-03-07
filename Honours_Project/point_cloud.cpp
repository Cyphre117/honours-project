#include "point_cloud.h"
#include <vector>
#include <gtc/type_ptr.hpp>
#include "vr_system.h"

PointCloud::PointCloud() :
	vao_(0),
	vbo_(0),
	num_verts_(0)
{
}

PointCloud::~PointCloud()
{

}

bool PointCloud::init()
{
	shader_.loadVertexSourceFile( "colour_shader_vs.glsl" );
	shader_.loadFragmentSourceFile( "colour_shader_fs.glsl" );
	shader_.init();

	modl_matrix_location_ = shader_.getUniformLocation( "model" );
	view_matrix_location_ = shader_.getUniformLocation( "view" );
	proj_matrix_location_ = shader_.getUniformLocation( "projection" );

	shader_.bind();

	glGenVertexArrays( 1, &vao_ );
	glGenBuffers( 1, &vbo_ );
	glBindVertexArray( vao_ );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_ );

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	GLint posAttrib = shader_.getAttribLocation( "vPosition" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	offset += sizeof( GLfloat ) * 3;
	GLint colAttrib = shader_.getAttribLocation( "vColour" );
	glEnableVertexAttribArray( colAttrib );
	glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	std::vector<GLfloat> data;

	for( int i = 0; i < 500; i++ )
	{
		data.push_back( (std::rand() / (float)RAND_MAX) * 5.0f - 2.5f );
		data.push_back( (std::rand() / (float)RAND_MAX) * 5.0f - 2.5f );
		data.push_back( (std::rand() / (float)RAND_MAX) * 5.0f - 2.5f );
		data.push_back( 1.0f );
		data.push_back( 1.0f );
		data.push_back( 1.0f );
	}

	// Send the verticies
	glBufferData( GL_ARRAY_BUFFER, sizeof( data[0] ) * data.size(), data.data(), GL_STATIC_DRAW );
	num_verts_ = data.size() / 6;

	glBindVertexArray( 0 );

	return false;
}

void PointCloud::render( vr::EVREye eye )
{
	VRSystem* system = VRSystem::get();

	view_mat_ = system->viewMatrix( eye );
	projection_mat_ = system->projectionMartix( eye );

	shader_.bind();
	glUniformMatrix4fv( modl_matrix_location_, 1, GL_FALSE, glm::value_ptr( model_mat_ ) );
	glUniformMatrix4fv( view_matrix_location_, 1, GL_FALSE, glm::value_ptr( view_mat_ ) );
	glUniformMatrix4fv( proj_matrix_location_, 1, GL_FALSE, glm::value_ptr( projection_mat_ ) );

	glBindVertexArray( vao_ );
	glDrawArrays( GL_POINTS, 0, num_verts_ );
}