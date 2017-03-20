#pragma once
#include <SDL.h>
#include <GL/glew.h>

#include "shader_program.h"

/* SINGLETON */
class Window
{
public:
	// Returns a pointer to the window, or nullptr on failure
	static Window* get();
	~Window();

	void render( GLuint left_eye_texture, GLuint right_eye_texture );
	inline void present() { SDL_GL_SwapWindow( win_ ); }

	// Getters
	int width() { return width_; }
	int height() { return height_; }
	SDL_Window* SDLWindow() { return win_; }
	SDL_GLContext SDLGLContext() { return gl_context_; }

private:
	Window();
	static Window* self_;
	bool init();

	SDL_Window* win_;
	SDL_GLContext gl_context_;
	int width_;
	int height_;

	ShaderProgram window_shader_;
	GLuint vao_;
};