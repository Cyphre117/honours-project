#include "pointer_tool.h"
#include "shader_program.h"
#include "vr_system.h"
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>

PointerTool::PointerTool() :
	VRTool( VRToolType::Pointer )
{}

PointerTool::~PointerTool()
{}

bool PointerTool::init()
{
	initialised_ = true;
	bool success = true;

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

	sphere_.setRadius( 0.04f );
	sphere_.init();

	return success;
}

void PointerTool::shutdown()
{
	if( vao_ )
	{
		glDeleteVertexArrays( 1, &vao_ );
		vao_ = 0;
	}
	if( vbo_ )
	{
		glDeleteBuffers( 1, &vbo_ );
		vbo_ = 0;
	}
}

void PointerTool::activate()
{
	active_ = true;
	sphere_.setActive( true );
}

void PointerTool::deactivate()
{
	active_ = false;
	sphere_.setActive( false );
}

void PointerTool::update( float dt )
{
	if( controller_ && controller_->isButtonDown( vr::k_EButton_SteamVR_Touchpad ) )
	{
		glBindVertexArray( vao_ );
		glBindBuffer( GL_ARRAY_BUFFER, vbo_ );
		float length = -0.45f + -0.4f * controller_->axis( vr::k_EButton_Axis0 ).y;

		std::vector<GLfloat> verts = {
			0,0,0, 1,1,1,
			0,0,length, 1,0,1
		};

		glBufferData( GL_ARRAY_BUFFER, sizeof( verts[0] ) * verts.size(), verts.data(), GL_STREAM_DRAW );
		num_verts_ = verts.size() / 6;

		sphere_.setActive( true );
		sphere_.setPosition( { 0, 0, length } );
		sphere_.update( dt );
	}
	else
	{
		sphere_.setActive( false );
		num_verts_ = 0;
	}
}

void PointerTool::render( const glm::mat4& view, const glm::mat4& projection )
{
	shader_->bind();
	glUniformMatrix4fv( shader_view_mat_location_, 1, GL_FALSE, glm::value_ptr( view ) );
	glUniformMatrix4fv( shader_proj_mat_location_, 1, GL_FALSE, glm::value_ptr( projection ) );
	glUniformMatrix4fv( shader_modl_mat_location_, 1, GL_FALSE, glm::value_ptr( controller_->deviceToAbsoluteTracking() ) );

	glBindVertexArray( vao_ );
	glDrawArrays( GL_LINES, 0, num_verts_ );

	sphere_.setParentTransform( controller_->deviceToAbsoluteTracking() );
	sphere_.render( view, projection );
}