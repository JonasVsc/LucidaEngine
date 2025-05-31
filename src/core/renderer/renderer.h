#pragma once

struct EngineContext;

class Renderer {
public:
	Renderer(EngineContext& ctx);

	~Renderer();

private:

	EngineContext& m_context;
};