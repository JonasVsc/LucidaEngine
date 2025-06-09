#include "lucida/lucida.h"
#include <assert.h>

int main(int argc, char **argv) 
{
	// Create Window
	LeWindowCreateInfo windowCI{};
	windowCI.title = "Lucida Window";
	windowCI.extent = { 640, 480 };

	LeWindow window{};
	leWindowCreate(&windowCI, &window);

	// Create Renderer
	LeRendererCreateInfo rendererCI{};
	rendererCI.pWindow = &window;

	LeRenderer renderer{};
	leRendererInit(&rendererCI, &renderer);

	while(!window.shouldClose)
	{
		leWindowUpdate(&window);
	}

	leRendererDestroy(&renderer);
	leWindowDestroy(&window);
	return 0; 
}