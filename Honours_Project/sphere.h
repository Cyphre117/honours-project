#pragma once

#include "GL/glew.h"
#include <glm.hpp>
#include <openvr.h>

class ShaderProgram;

class Sphere
{
public:
	Sphere();
	~Sphere();

	void init();
	void shutdown();

	void update( float dt );
	void render( glm::mat4 view, glm::mat4 projection );
	bool isTouching( const Sphere& other ) const;

	// Setters
	void setRadius( float radius ) { radius_ = radius; }
	void setSegments( int segments ) { segments_ = segments; }
	void setColour( float r, float g, float b ) { colour_ = { r, g, b }; }
	void setPosition( glm::vec3 position ) { position_ = position; }
	void setActive( bool active ) { active_ = active; }
	void setParentTransform( glm::mat4 transform ) { parent_transform_ = transform; }
	static void setShader( ShaderProgram* shader ) { shader_ = shader; }

	// Getters
	float radius() const { return radius_; }
	int segments() const { return segments_; }
	glm::vec3 position() const { return position_; }

protected:
	bool active_ = true;
	float radius_ = 0.1f;
	int segments_ = 20;
	glm::vec3 colour_ = { 1.0f, 0.0f, 1.0f };
	glm::vec3 position_ = { 0.0f, 0.0f, 0.0f };
	glm::mat4 parent_transform_ = glm::mat4( 1.0f );

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLsizei num_verts_ = 0;
	GLint shader_view_mat_location_ = 0;
	GLint shader_proj_mat_location_ = 0;
	GLint shader_modl_mat_location_ = 0;

	static ShaderProgram* shader_;
};