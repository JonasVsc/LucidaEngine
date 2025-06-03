#include "renderer.h"

// core
#include "core/log.h"

Renderer::Renderer(Config& config, Window& window)
	: m_config{config}
	, m_window{window}
{
	jinfo("renderer constructor");
}

Renderer::~Renderer()
{
	jinfo("renderer destructor");
}
