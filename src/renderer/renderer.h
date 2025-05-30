#pragma once

// lucida
#include "config/lucida_config.h"

// lib
#include <fmt/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// std
#include <vector>

#define VK_CHECK(x)																							\
	do																										\
	{																										\
		VkResult err = x;																					\
		if (err)																							\
		{																									\
		   fmt::println("\033[38;2;255;128;128;4;5m Detected Vulkan error: {}\033[0m", string_VkResult(err));	\
			abort();																						\
		}																									\
	} while (0)

class Window;

class Renderer {
public:
	Renderer(Window& window, LucidaConfig& lc);

	~Renderer();

	// Destroy Renderer
	void destroy_renderer();

private:
	Window& m_rWindow;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};