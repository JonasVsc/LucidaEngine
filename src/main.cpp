#include "window.h"
#include "renderer.h"

// GLOBALS


int main(int argc, char **argv) 
{
	LeWindow window;
	LeRenderer renderer;

	LeWindowCreateInfo windowCI{ "Lucida Game", 640, 480 };
	leWindowCreate(windowCI, window);
	leRendererCreate(window, renderer);

	while (!window.shouldClose)
	{
		leWindowPoolEvents(window);
	}

	leRendererDestroy(renderer);
	leWindowDestroy(window);
	return 0; 
}