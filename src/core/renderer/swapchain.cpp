#include "swapchain.h"

// core
#include "core/window/window.h"
#include "device.h"
#include "core/context.h"
#include "core/log.h"

// std
#include <algorithm>

Swapchain::Swapchain(EngineContext& ctx)
	: m_context{ctx}
{
	jinfo("swapchain constructor");
	create_swapchain();
	create_swapchain_image_views();
}

Swapchain::~Swapchain()
{
	jinfo("swapchain destructor");
	for (auto img : m_image_views)
	{
		vkDestroyImageView(m_context.device->get_handle(), img, nullptr);
	}
	vkDestroySwapchainKHR(m_context.device->get_handle(), m_swapchain, nullptr);
}

void Swapchain::create_swapchain()
{
	SwapchainSupportDetails swapchain_support = m_context.device->query_swapchain_support_details();

	VkSurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
	VkPresentModeKHR present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
	VkExtent2D extent = choose_swapchain_extent(swapchain_support.capabilities);

	uint32_t count_image = swapchain_support.capabilities.minImageCount + 1;

	if (swapchain_support.capabilities.maxImageCount > 0 && count_image > swapchain_support.capabilities.maxImageCount)
	{
		count_image = swapchain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_context.device->get_surface(),
		.minImageCount = count_image,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = swapchain_support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.oldSwapchain = VK_NULL_HANDLE
	};

	QueueFamilyIndices indices = m_context.device->find_queue_families();
	uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };
	if (indices.is_exclusive())
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	}

	VK_CHECK(vkCreateSwapchainKHR(m_context.device->get_handle(), &swapchain_create_info, nullptr, &m_swapchain));

	vkGetSwapchainImagesKHR(m_context.device->get_handle(), m_swapchain, &count_image, nullptr);
	m_images.resize(count_image);
	vkGetSwapchainImagesKHR(m_context.device->get_handle(), m_swapchain, &count_image, nullptr);
	vkGetSwapchainImagesKHR(m_context.device->get_handle(), m_swapchain, &count_image, m_images.data());

	m_image_format = surface_format.format;
	m_extent = extent;
}

void Swapchain::create_swapchain_image_views()
{
	m_image_views.resize(m_images.size());

	for (int i = 0; i < m_images.size(); i++)
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_image_format,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		VK_CHECK(vkCreateImageView(m_context.device->get_handle(), &image_view_create_info, nullptr, &m_image_views[i]));
	}
}


VkSurfaceFormatKHR Swapchain::choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& availableFormat : available_formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return available_formats[0];
}

VkPresentModeKHR Swapchain::choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& present_mode : available_present_modes) {
		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return present_mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilites)
{
	if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilites.currentExtent;
	}
	int width, height;
	SDL_Vulkan_GetDrawableSize(m_context.window->w_sdl(), &width, &height);

	VkExtent2D actual_extent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actual_extent.width = std::clamp(actual_extent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
	actual_extent.height = std::clamp(actual_extent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);

	return actual_extent;
}