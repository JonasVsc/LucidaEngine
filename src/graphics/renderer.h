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

private:

	Config& m_config;
	Window& m_window;

	Device m_device{ m_config, m_window };
	Swapchain m_swapchain{ m_window, m_device };


};