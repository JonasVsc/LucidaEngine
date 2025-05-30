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

struct VulkanContext {
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
};

class Window;

class Renderer {
public:
	Renderer(Window& window, LucidaConfig& lc);

	~Renderer();

private:

	void create_instance();
	void select_physical_device();

	Window& m_window;
	LucidaConfig& m_config;
	VulkanContext m_context;
};