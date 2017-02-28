#pragma once
#include <SDL.h>
#include <GL/glew.h>

/* SINGLETON */
class Window
{
public:
	// Returns a pointer to the window, or nullptr on failure
	static Window* get();
	~Window();

	inline void present() { SDL_GL_SwapWindow( win_ ); }

	// Getters
	int width() { return width_; }
	int height() { return height_; }

protected:
	Window();
	static Window* self_;
	bool init();

	SDL_Window* win_;
	SDL_GLContext gl_context_;
	int width_;
	int height_;
};