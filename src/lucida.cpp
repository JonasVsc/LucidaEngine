#include "lucida.h"

// core
#include "core/log.h"

#include "engine/engine.h"

int main(int argc, char** argv)
{
	Config config{ "lucida.json" };
	Engine my_engine{ config };

	try {
		my_engine.run();
	}
	catch (std::exception& e)
	{
		jerr("exception: {}", e.what());
	}

	return 0;
}