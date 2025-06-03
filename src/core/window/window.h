#pragma once

// lib
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

class Config;

class Window {
public:

	Window(Config& lc);

	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete; 

	// Process window events
	void process_events();

	// Returns false if window was closed
	bool closed() const { return m_closed; }

	SDL_Window* w_sdl() const // accessor
	{
		return m_window;
	}

private:

	SDL_Window* m_window = nullptr;
	bool m_closed = false;
};

