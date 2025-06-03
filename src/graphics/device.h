#pragma once

// lib
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

// std
#include <vector>
#include <optional>

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

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

class Config;
class Window;

class Device {

	const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

public:
	Device(Config& config, Window& window);

	~Device();

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(Device&&) = delete;

	SwapchainSupportDetails query_swapchain_support_details();
	QueueFamilyIndices find_queue_families();

	VkSurfaceKHR get_surface() const { return m_surface; }
	VkDevice get_handle() const { return m_device; }

private:

	void create_instance();
	void select_physical_device();
	void create_device();
	void create_allocator();
	bool is_physical_device_suitable(VkPhysicalDevice physical_device);
	bool check_device_extension_support(VkPhysicalDevice device);
	int rate_physical_device_suitability(VkPhysicalDevice physical_device);
	QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device);
	SwapchainSupportDetails query_swapchain_support_details(VkPhysicalDevice physical_device);

	Config& m_config;
	Window& m_window;

	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physical_device;
	VkPhysicalDeviceProperties m_physical_device_properties;
	VkDevice m_device;
	VkQueue m_graphics_queue;
	VkQueue m_present_queue;
	VmaAllocator m_allocator;
};