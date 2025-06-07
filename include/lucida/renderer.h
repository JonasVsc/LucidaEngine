#pragma once

#include "core.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#define VALIDATION_LAYERS true

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

struct LeRendererCreateInfo {
    LeWindow* pWindow;
};

struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
};

struct LeRenderer {
    VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VmaAllocator allocator;
	QueueFamilyIndices indices;
    LeWindow* pWindow;
};

LeResult leRendererInit(const LeRendererCreateInfo* pCreateInfo, LeRenderer* pRenderer);
void leRendererShutdown(LeRenderer* pRenderer);