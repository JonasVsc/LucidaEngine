#include "lucida.h"

#include "core/config/config.h"
#include "core/window/window.h"
// Renderer
#include "core/renderer/device.h"
#include "core/renderer/swapchain.h"
#include "core/renderer/renderer.h"
#include "core/context.h"
#include "core/log.h"

int main(int argc, char** argv)
{
	EngineContext context;

	Config config("lucida.json");
	context.config = &config;

	Window window{ config };
	context.window = &window;
	
	Device device{ context };
	context.device = &device;

	Swapchain swapchain{ context };
	context.swapchain = &swapchain;

	Renderer renderer{ context };

	try {
		while (!window.closed())
		{
			window.process_events();
		}
	}
	catch (std::exception& e)
	{
		fmt::println("exception: {}", e.what());
	}

	return 0;
}