#include "engine.h"

// core
#include "core/log.h"

#include "graphics/shader.h"
#include "graphics/pipeline.h"


Engine::Engine(Config& config)
	: m_config{config}
{
	jinfo("engine constructor");

	// create shader modules
	Shader my_vert_shader{ m_renderer.get_device(), "shaders/spv/test.vert.spv" };
	Shader my_frag_shader{ m_renderer.get_device(), "shaders/spv/test.frag.spv" };

	Pipeline my_pipeline = PipelineBuilder::create(m_renderer.get_pipeline_layout(), m_renderer.get_render_pass())
		.add_shader_stage(my_vert_shader.get_module(), VK_SHADER_STAGE_VERTEX_BIT)
		.add_shader_stage(my_frag_shader.get_module(), VK_SHADER_STAGE_FRAGMENT_BIT)
		.set_dynamic_states(VK_DYNAMIC_STATE_VIEWPORT)
		.set_dynamic_states(VK_DYNAMIC_STATE_SCISSOR)
		.add_color_blend_attachment()
		.build(m_renderer.get_device());
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
