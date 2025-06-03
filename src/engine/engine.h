#pragma once

// core
#include "core/config/config.h"

#include "window/window.h"
#include "graphics/renderer.h"

class Engine {
public:

	Engine(Config& config);

	~Engine();

	void run();

private:

	Config& m_config;
	
	Window m_window{m_config};

	Renderer m_renderer{ m_config, m_window };
};