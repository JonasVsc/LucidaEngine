#include "engine.h"

// core
#include "core/log.h"


Engine::Engine(Config& config)
	: m_config{config}
{
	jinfo("engine constructor");
}

Engine::~Engine()
{
	jinfo("engine destructor");
}

void Engine::run()
{
	while (!m_window.closed())
	{
		m_window.process_events();
	}
}
