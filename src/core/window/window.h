#pragma once

// lib
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

class Config;

class Window {
public:

	Window(Config& lc);

	~Window();

	Window(const Window& w) = delete;
	Window& operator=(const Window& other) = delete;
	Window(Window&& w) = delete;
	Window& operator=(Window&& other) = delete; 

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

