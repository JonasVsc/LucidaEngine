#pragma once
#include "core.h"
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>

struct LeExtent {
	int width;
	int height;
};

struct LeWindowCreateInfo {
	const char *title;
	LeExtent extent;
};

struct LeWindow {
	SDL_Window* handle;
	bool shouldClose;
	LeExtent extent;
};

// Init window
LeResult leWindowCreate(const LeWindowCreateInfo *pCreateInfo, LeWindow *pOutWindow);

// Update window
void leWindowUpdate(LeWindow* pWindow);

// Destroy window
void leWindowDestroy(LeWindow* pWindow);