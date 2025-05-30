#pragma once

// lucida
#include "config/lucida_config.h"

// lib
#include <fmt/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// std
#include <vector>
#include <optional>

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

struct QueueFamilyIndices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool is_complete()
	{
		return graphics_family.has_value() && present_family.has_value();
	}

	bool is_exclusive()
	{
		return graphics_family.value() == present_family.value();
	}
};

struct VulkanContext {
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphics_queue = VK_NULL_HANDLE;
	VkQueue present_queue = VK_NULL_HANDLE;

	// Utility
	VkPhysicalDeviceProperties physical_device_properties;
};

class Window;

class Renderer {

	const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

public:
	Renderer(Window& window, LucidaConfig& lc);

	~Renderer();

private:

	void create_instance();
	void select_physical_device();
	void create_device();

	QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device);
	bool is_physical_device_suitable(VkPhysicalDevice physical_device);
	bool check_device_extension_support(VkPhysicalDevice device);


	Window& m_window;
	LucidaConfig& m_config;
	VulkanContext m_context;
};