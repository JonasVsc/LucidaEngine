#pragma once
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>

struct LeWindowCreateInfo {
	const char *title;
	int width;
	int height;
};

struct LeWindow {
	SDL_Window* handle;
	bool shouldClose;
	int width;
	int height;
};

// Init window
void leWindowCreate(const LeWindowCreateInfo& createInfo, LeWindow &window);

// Pool events
void leWindowPoolEvents(LeWindow& window);

// Destroy window
void leWindowDestroy(LeWindow& window);