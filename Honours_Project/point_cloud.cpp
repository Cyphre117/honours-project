#include "point_cloud.h"
#include <iostream>
#include <vector>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "vr_system.h"

PointCloud::PointCloud() :
	active_shader_(nullptr),
	vao_(0),
	vbo_(0),
	num_verts_(0),
	aabb_vao_(0),
	model_mat_(),
	offset_mat_()
{
}

PointCloud::~PointCloud()
{
}

bool PointCloud::init()
{
	ply_loader_.load( "models/dragon_res2.ply", data_ );

	modl_matrix_location_ = active_shader_->getUniformLocation( "model" );
	view_matrix_location_ = active_shader_->getUniformLocation( "view" );
	proj_matrix_location_ = active_shader_->getUniformLocation( "projection" );

	calculateAABB();
	resetPosition();

	glGenVertexArrays( 1, &vao_ );
	glGenBuffers( 1, &vbo_ );
	glBindVertexArray( vao_ );
	glBindBuffer( GL_ARRAY_BUFFER, vbo_ );

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	offset += sizeof( GLfloat ) * 3;
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	// Send the verticies
	glBufferData( GL_ARRAY_BUFFER, sizeof( data_[0] ) * data_.size(), data_.data(), GL_STATIC_DRAW );
	num_verts_ = (GLsizei)(data_.size() / 6);

	glBindVertexArray( 0 );
	
	return true;
}

void PointCloud::render( vr::EVREye eye )
{
	VRSystem* system = VRSystem::get();

	view_mat_ = system->viewMatrix( eye );
	projection_mat_ = system->projectionMartix( eye );

	active_shader_->bind();
	glUniformMatrix4fv( modl_matrix_location_, 1, GL_FALSE, glm::value_ptr( model_mat_ * offset_mat_ ) );
	glUniformMatrix4fv( view_matrix_location_, 1, GL_FALSE, glm::value_ptr( view_mat_ ) );
	glUniformMatrix4fv( proj_matrix_location_, 1, GL_FALSE, glm::value_ptr( projection_mat_ ) );

	glBindVertexArray( vao_ );
	glDrawArrays( GL_POINTS, 0, num_verts_ );

	// Draw the aabb vao;
	glBindVertexArray( aabb_vao_ );
	glDrawArrays( GL_LINES, 0, 24 );
}

void PointCloud::calculateAABB()
{
	lower_bound_ = glm::vec3( 0, 0, 0 );
	upper_bound_ = glm::vec3( 0, 0, 0 );

	// find the min and max XYZ values
	for( size_t i = 0; i < data_.size(); i += 6 )
	{
		// the data_ vector takes the form XYZRGB, so offset 0, 1, 2 are XYZ
		if( lower_bound_.x < data_[i + 0] ) lower_bound_.x = data_[i + 0];
		if( lower_bound_.y < data_[i + 1] ) lower_bound_.y = data_[i + 1];
		if( lower_bound_.z < data_[i + 2] ) lower_bound_.z = data_[i + 2];

		if( upper_bound_.x > data_[i + 0] ) upper_bound_.x = data_[i + 0];
		if( upper_bound_.y > data_[i + 1] ) upper_bound_.y = data_[i + 1];
		if( upper_bound_.z > data_[i + 2] ) upper_bound_.z = data_[i + 2];
	}

	GLuint vbo;

	glGenVertexArrays( 1, &aabb_vao_ );
	glBindVertexArray( aabb_vao_ );
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );

	GLuint stride = 2 * 3 * sizeof( GLfloat );
	GLuint offset = 0;

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	offset += sizeof( GLfloat ) * 3;
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset );

	// Construct a box around the point cloud
	GLfloat verts[] = {
		// lines traveling on the y axis (vertical)
		lower_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,
		lower_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,

		upper_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,
		upper_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,

		lower_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,
		lower_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,

		upper_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,
		upper_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,
		
		// lines traveling on the x axis
		lower_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,
		upper_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,

		lower_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,
		upper_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,

		lower_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,
		upper_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,

		lower_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,
		upper_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,

		// lines traveling on the z axis
		lower_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,
		lower_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,

		lower_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,
		lower_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,

		upper_bound_.x, lower_bound_.y, lower_bound_.z, 1, 1, 1,
		upper_bound_.x, lower_bound_.y, upper_bound_.z, 1, 1, 1,

		upper_bound_.x, upper_bound_.y, lower_bound_.z, 1, 1, 1,
		upper_bound_.x, upper_bound_.y, upper_bound_.z, 1, 1, 1,
	};

	glBufferData( GL_ARRAY_BUFFER, sizeof( verts ), verts, GL_STATIC_DRAW );
}

void PointCloud::resetPosition()
{
	float scale = 0.6f / std::abs( upper_bound_.x - lower_bound_.x );
	glm::vec3 position( 0, 0.9, -0.3 - std::abs( upper_bound_.z - lower_bound_.z ) * 0.5f * scale );
	glm::vec3 center = lower_bound_ + (upper_bound_ - lower_bound_) * 0.5f;
/*
	std::cout << "Lower bound " << lower_bound_.x << " " << lower_bound_.y << " " << lower_bound_.z << std::endl;
	std::cout << "Upper bound " << upper_bound_.x << " " << upper_bound_.y << " " << upper_bound_.z << std::endl;
	std::cout << "scale amount " << scale << std::endl;
	std::cout << "center " << center.x << " " << center.y << " " << center.z << std::endl;
*/
	model_mat_ = glm::mat4();
	model_mat_ = glm::translate( model_mat_, position );
	model_mat_ = glm::scale( model_mat_, glm::vec3( scale, scale, scale ) );
	model_mat_ = glm::translate( model_mat_, -center );
}