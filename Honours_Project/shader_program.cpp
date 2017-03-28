#include "shader_program.h"
#include <fstream>
#include <sstream>

// Shader base path is found by SDL the first time it is needed
std::string ShaderProgram::shader_base_path_ = "";
// The folder to look in relative to the base path
#define SHADER_FOLDER "shaders"

#ifdef _WIN32
    #define PATH_SEPERATOR '\\'
#else
    #define PATH_SEPERATOR '/'
#endif

ShaderProgram::ShaderProgram() :
vertex_shader_(0),
fragment_shader_(0)
{}

void ShaderProgram::init( std::string vertex_file_path, std::string fragment_file_path )
{
	loadVertexSourceFile( vertex_file_path );
	loadFragmentSourceFile( fragment_file_path );
	init();
}

bool ShaderProgram::init()
{
    bool success = true;
    const GLchar* source_ptr = nullptr;

    if( !vertex_source_.empty() )
    {
    	vertex_shader_ = glCreateShader( GL_VERTEX_SHADER );
        if( vertex_shader_ == 0 )
        {
            SDL_Log( "ERROR could not create vertex shader" );
            success = false;
        }

        // Shader strings have to be converted to const GLchar* so OpenGL can compile them
        source_ptr = (const GLchar*)vertex_source_.c_str();
        
        glShaderSource( vertex_shader_, 1, &source_ptr, NULL );
        glCompileShader( vertex_shader_ );

        if( did_shader_compile_ok( vertex_shader_ ) )
            vertex_source_.resize(0);
        else
            success = false;
    }
    else
    {
        SDL_Log( "ERROR vertex shader source was empty! (not set)" );
    }

    if( !fragment_source_.empty() )
    {
        // Create the fragment shader
    	fragment_shader_ = glCreateShader( GL_FRAGMENT_SHADER );
        if( fragment_shader_ == 0 )
        {
            SDL_Log( "ERROR could not create fragment shader" );
            success = false;
        }

        // Shader strings have to be converted to const GLchar* so OpenGL can compile them
        source_ptr = (const GLchar*)fragment_source_.c_str();
        
        glShaderSource( fragment_shader_, 1, &source_ptr , NULL );
        glCompileShader( fragment_shader_ );

        if( did_shader_compile_ok( fragment_shader_ ) )
            fragment_source_.resize(0);
        else
            success = false;
    }
    else
    {
        SDL_Log( "ERROR fragment shader source was empty! (not set)" );
    }

    if( success ) {
    	 // Create the shader program, attach the vertex and fragment shaders
	    program_ = glCreateProgram();
	    glAttachShader( program_, vertex_shader_ );
	    glAttachShader( program_, fragment_shader_ );
	    glBindFragDataLocation( program_, 0, "outColour" );
	    glLinkProgram( program_ );

        // now cleanup the shaders
        // TODO: it's possible we want to the same vertex/fagment/etc. shader multple times in different programs
        // TODO: find a way of supporting this without the caller having to do extra work, on the other hand it might not be worth it
        glDetachShader( program_, vertex_shader_ );
        glDetachShader( program_, fragment_shader_ );
        glDeleteShader( vertex_shader_ );
        glDeleteShader( fragment_shader_ ); 
    }

    // Return true on success, false on error
    return success;
}

void ShaderProgram::shutdown()
{
    glDeleteProgram( program_ );
    glDeleteShader( vertex_shader_ );
    glDeleteShader( fragment_shader_ );
}

void ShaderProgram::bind() const
{
	glUseProgram( program_ );
}

GLint ShaderProgram::getUniformLocation( const GLchar* name ) const
{
    GLint uniform = glGetUniformLocation( program_, name );
    if( uniform == -1 ) {
        SDL_Log("ERROR did not find uniform '%s' in shader program '%i'", name, program_ );
    }
    return uniform;
}


GLint ShaderProgram::getAttribLocation( const GLchar* name ) const
{
    GLint attribute = glGetAttribLocation( program_, name );
    if( attribute == -1 ) {
        SDL_Log("ERROR did not find attribute '%s' in shader program '%i'", name, program_ );
    }
    return attribute;
}

bool ShaderProgram::loadVertexSourceFile( std::string file_path )
{
    return load_file( file_path, &vertex_source_ );
}

bool ShaderProgram::loadFragmentSourceFile( std::string file_path )
{
    return load_file( file_path, &fragment_source_ );
}

// TODO: convert this to SDL rw ops and move to seperate header file
bool ShaderProgram::load_file( std::string filename, std::string* file_contents )
{
    // Use SDL to get the base resource path the first time only;
    if( shader_base_path_ == "" )
    {
        get_shader_base_path();
    }
    
    std::ifstream file( shader_base_path_ + filename );
    if( !file.good() )
    {
        SDL_Log("Could not load  %s!", filename.c_str() );
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    *file_contents = buffer.str();

    return true;
}

bool ShaderProgram::did_shader_compile_ok( GLuint shader )
{    
    // Check the shader was compiled succesfully
    GLint status;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE )
    {
        // Get the length of the error log
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        // Now get the error log itself
        GLchar* buffer = new GLchar[log_length];
        glGetShaderInfoLog( shader, log_length, NULL, buffer );

        // Print the error
        // TODO: if we have the source then could we print the line that was broken?
        SDL_Log( "ERROR: compiling shader..." );
        SDL_Log( "%s", buffer );
		delete[] buffer;
        return false;
    }

    return true;
}

void ShaderProgram::get_shader_base_path()
{
    // Use SDL to get the base path of the exe
    char* charPath = SDL_GetBasePath();

    // Add the shader folder to the project base path and a seperator on the end
    shader_base_path_ = std::string(charPath) + SHADER_FOLDER + PATH_SEPERATOR;

    // Free the SDL allocated string
    SDL_free(charPath);
}