#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <string>

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram() {}

	// Returns true if shader compilation was a success, false if there was an error
	bool init();
	void shutdown();

	// Returns true if the source files were loaded successfully, false if they could not be loaded
	bool loadVertexSourceFile( std::string file_path );
	bool loadFragmentSourceFile( std::string file_path ); 

	// Sets the shader source strings directly
	void setVertexSourceString( const std::string& source ) { vertex_source_ = source; }
	void setFragmentSourceString( const std::string& source ) { fragment_source_ = source; }

	// Bind the shader to the OpenGL context, ready for use
	void bind();

	// Don't forget to bind shaders before trying to get uniforms or attributes
	GLint getUniformLocation( const GLchar* name );
	GLint getAttribLocation( const GLchar* name );
	GLint getProgram() { return program_; }

private:
	// Loads the text file 'filename' and passes the contents to the pointer
	bool load_file( std::string filename, std::string* file_contents  );
	bool did_shader_compile_ok( GLuint shader );
	void get_shader_base_path();

	std::string vertex_source_;
	std::string fragment_source_;

	GLuint program_;
	GLuint vertex_shader_;
	GLuint fragment_shader_;

	static std::string shader_base_path_;
	//static const std::string SHADER_FOLDER_;
	//static const char PATH_SEPERATOR_;
};

#endif