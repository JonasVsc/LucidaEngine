#pragma once

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#define VALIDATION_LAYERS true

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#define VK_CHECK(x)																										\
	do																													\
	{																													\
		VkResult err = x;																								\
		if (err)																										\
		{																												\
		    fprintf(stderr, "\033[38;2;255;128;128;4;5m Detected Vulkan error: %s\033[0m", string_VkResult(err));		\
			abort();																									\
		}																												\
	} while (0)

// forward declarations
struct LeWindow;


struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	VkPresentModeKHR* presentModes;
};

struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};

struct LeDevice {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	SwapchainSupportDetails details;
	VkDevice handle;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	QueueFamilyIndices queueIndices;
	VmaAllocator allocator;
};

struct LeSwapchain {
	VkSwapchainKHR handle;
	uint32_t imageCount;
	VkImage* image;
	VkImageView* imageView;
	VkSurfaceFormatKHR format;
	VkExtent2D extent;
};

struct LeRendererCreateInfo {
    LeWindow* pWindow;
};

struct LeRenderer {
	LeDevice device;
	LeSwapchain swapchain;
    LeWindow* pWindow;

	VkRenderPass renderPass;
	VkFramebuffer* pFramebuffers;
	uint32_t framebufferCount;

	VkCommandPool commandPool;
	VkCommandBuffer* commandBuffers;
	uint32_t commandBufferCount;

	// Sync Objects
	VkFence* pInFlightFences;
	VkFence* pImageInFlightFences;
	VkSemaphore* pImageAvailableSemaphores;
	VkSemaphore* pRenderFinishedSemaphores;
	uint32_t syncObjectsCount;
};

LeResult leRendererInit(const LeRendererCreateInfo* pCreateInfo, LeRenderer* pRenderer);
void leRendererDestroy(LeRenderer* pRenderer);