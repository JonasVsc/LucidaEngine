#include "renderer.h"

#include "core/context.h"
#include "core/log.h"

Renderer::Renderer(EngineContext& ctx)
	: m_context{ctx}
{
	jinfo("renderer constructor");
}

Renderer::~Renderer()
{
	jinfo("renderer destructor");
}
