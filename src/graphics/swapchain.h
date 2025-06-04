#pragma once

// lib
#include <vulkan/vulkan.h>

// std
#include <vector>

class Device;
class Window;

class Swapchain {
public:

	Swapchain(Window& window, Device& device);

	~Swapchain();

	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;

	VkFormat get_image_format() { return m_image_format; }

private:
	
	void create_swapchain();
	void create_swapchain_image_views();
	
	VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
	VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilites);

	Window& m_window;
	Device& m_device;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_image_views;
	VkFormat m_image_format;
	VkExtent2D m_extent;

};