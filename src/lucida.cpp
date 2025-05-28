#include "lucida.h"


int main(int argc, char** argv)
{
	LucidaConfig config("lucida.json");
	
	Window window{ config };

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