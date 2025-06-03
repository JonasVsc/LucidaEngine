#include "lucida.h"

#include "core/config/config.h"
#include "core/window/window.h"
#include "core/renderer/renderer.h"
#include "core/log.h"

int main(int argc, char** argv)
{
	Config config{ "lucida.json" };
	Window window{ config };
	Renderer renderer{ config, window };

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