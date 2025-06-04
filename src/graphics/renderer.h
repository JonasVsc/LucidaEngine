#pragma once

#include "device.h"
#include "swapchain.h"

class Config;
class Window;

class Renderer {
public:

	Renderer(Config& config, Window& window);

	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	Device& get_device() { return m_device; }
	VkRenderPass get_render_pass() const { return m_render_pass; }
	VkPipelineLayout get_pipeline_layout() const { return m_pipeline_layout; }
private:

	void create_render_pass();
	void create_pipeline_layout();

	Config& m_config;
	Window& m_window;

	Device m_device{ m_config, m_window };
	Swapchain m_swapchain{ m_window, m_device };

	// temporary
	VkRenderPass m_render_pass;
	VkPipelineLayout m_pipeline_layout;

};