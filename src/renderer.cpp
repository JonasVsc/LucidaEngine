#include "lucida/renderer.h"
#include "lucida/window.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct LePhysicalDeviceSelectInfo {
	VkInstance instance;
	VkSurfaceKHR surface;
};

struct LeDeviceCreateInfo {
	VkPhysicalDevice physicalDevice;
	QueueFamilyIndices indices;
};

struct LeAllocatorCreateInfo {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
};

struct LeSwapchainCreateInfo {
	LeDevice device;
};

struct LeSwapchainImagesCreateInfo {
	VkDevice device;
	VkSwapchainKHR swapchain;
};

struct LeSwapchainImageViewCreateInfo {
	VkDevice device;
	uint32_t imageCount;
	const VkImage* pImages;
	VkSurfaceFormatKHR format;
};

// Create vulkan instance
static void leCreateInstance(const LeWindow* pWindow, VkInstance* pInstance);
static void leSelectPhysicalDevice(const LePhysicalDeviceSelectInfo* pSelectInfo, VkPhysicalDevice* pPhysicalDevice, QueueFamilyIndices* pIndices, SwapchainSupportDetails* pSwapchainDetails);
static void leCreateDevice(const LeDeviceCreateInfo* pCreateInfo, VkDevice* pDevice);
static void leCreateAllocator(const LeAllocatorCreateInfo* pCreateInfo, VmaAllocator* pAllocator);
static void leCreateSwapchain(const LeWindow* pWindow, const LeSwapchainCreateInfo* pCreateInfo, VkSwapchainKHR* pSwapchain, VkSurfaceFormatKHR* pFormat);
static void leCreateSwapchainImages(const LeSwapchainImagesCreateInfo* pCreateInfo, VkImage** ppImage, uint32_t* pImageCount);
static void leCreateSwapchainImageViews(const LeSwapchainImageViewCreateInfo* pCreateInfo, VkImageView** ppImageViews);

LeResult leRendererInit(const LeRendererCreateInfo* pCreateInfo, LeRenderer* pRenderer)
{
	leCreateInstance(pCreateInfo->pWindow, &pRenderer->device.instance);
	
	SDL_Vulkan_CreateSurface(pCreateInfo->pWindow->handle, pRenderer->device.instance, &pRenderer->device.surface);

	LePhysicalDeviceSelectInfo physicalDeviceSelectInfo{};
	physicalDeviceSelectInfo.instance = pRenderer->device.instance;
	physicalDeviceSelectInfo.surface = pRenderer->device.surface;
	
	leSelectPhysicalDevice(&physicalDeviceSelectInfo, &pRenderer->device.physicalDevice, &pRenderer->device.queueIndices, &pRenderer->device.details);

	LeDeviceCreateInfo deviceCI{};
	deviceCI.indices = pRenderer->device.queueIndices;
	deviceCI.physicalDevice = pRenderer->device.physicalDevice;
	
	leCreateDevice(&deviceCI, &pRenderer->device.handle);

	vkGetDeviceQueue(pRenderer->device.handle, pRenderer->device.queueIndices.graphicsFamily, 0, &pRenderer->device.graphicsQueue);
	vkGetDeviceQueue(pRenderer->device.handle, pRenderer->device.queueIndices.presentFamily, 0, &pRenderer->device.presentQueue);

	LeAllocatorCreateInfo allocatorCI{};
	allocatorCI.instance = pRenderer->device.instance;
	allocatorCI.physicalDevice = pRenderer->device.physicalDevice;
	allocatorCI.device = pRenderer->device.handle;

	leCreateAllocator(&allocatorCI, &pRenderer->device.allocator);

	LeSwapchainCreateInfo swapchainCI{};
	swapchainCI.device = pRenderer->device;

	leCreateSwapchain(pRenderer->pWindow, &swapchainCI, &pRenderer->swapchain.handle, &pRenderer->swapchain.format);

	LeSwapchainImagesCreateInfo swapchainImagesCI{};
	swapchainImagesCI.device = pRenderer->device.handle;
	swapchainImagesCI.swapchain = pRenderer->swapchain.handle;
	
	leCreateSwapchainImages(&swapchainImagesCI, &pRenderer->swapchain.image, &pRenderer->swapchain.imageCount);

	LeSwapchainImageViewCreateInfo swapchainImageViewsCI{};
	swapchainImageViewsCI.device = pRenderer->device.handle;
	swapchainImageViewsCI.imageCount = pRenderer->swapchain.imageCount;
	swapchainImageViewsCI.pImages = pRenderer->swapchain.image;
	swapchainImageViewsCI.format = pRenderer->swapchain.format;
	
	leCreateSwapchainImageViews(&swapchainImageViewsCI, &pRenderer->swapchain.imageView);
	
	return LE_SUCCESS;
}

void leRendererShutdown(LeRenderer* pRenderer)
{
	if (!pRenderer)
	{
		return;
	}

	free(pRenderer->device.details.formats);
	free(pRenderer->device.details.presentModes);
	
	for (uint32_t i = 0; i < pRenderer->swapchain.imageCount; ++i)
	{
		vkDestroyImageView(pRenderer->device.handle, pRenderer->swapchain.imageView[i], nullptr);
	}
	free(pRenderer->swapchain.imageView);

	vkDestroySwapchainKHR(pRenderer->device.handle, pRenderer->swapchain.handle, nullptr);
	vmaDestroyAllocator(pRenderer->device.allocator);
	vkDestroyDevice(pRenderer->device.handle, nullptr);
	vkDestroySurfaceKHR(pRenderer->device.instance, pRenderer->device.surface, nullptr);
	vkDestroyInstance(pRenderer->device.instance, nullptr);
}




static void leCreateInstance(const LeWindow* pWindow, VkInstance* instance)
{
	// SDL Extensions
	uint32_t sdlExtensionCount;
	SDL_Vulkan_GetInstanceExtensions(pWindow->handle, &sdlExtensionCount, nullptr);

	const char** pSDLExtensionsArray = (const char**)malloc(sdlExtensionCount * sizeof(const char**));
	SDL_Vulkan_GetInstanceExtensions(pWindow->handle, &sdlExtensionCount, pSDLExtensionsArray);
	assert(pSDLExtensionsArray != NULL && "heap allocation failed");

	// Instance Extensions
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	VkExtensionProperties* pExtensionPropertiesArray = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
	assert(pExtensionPropertiesArray != NULL && "heap allocation failed");

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, pExtensionPropertiesArray);

	// Check Extension Support
	for (uint32_t i = 0; i < sdlExtensionCount; ++i)
	{
		bool hasSupport = false;
		for (uint32_t j = 0; j < extensionCount; ++j)
		{
			if (!strcmp(pSDLExtensionsArray[i], pExtensionPropertiesArray[j].extensionName))
			{
				hasSupport = true;
				break;
			}
		}

		assert(hasSupport && "unsupported extensions found.");
	}

	// Create Instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

	VkInstanceCreateInfo instanceCI{};
	instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCI.pApplicationInfo = &appInfo;
	instanceCI.enabledExtensionCount = sdlExtensionCount;
	instanceCI.ppEnabledExtensionNames = pSDLExtensionsArray;

	if (VALIDATION_LAYERS)
	{
		const char* validationsLayers[] = {"VK_LAYER_KHRONOS_validation"};

		instanceCI.enabledLayerCount = 1;
		instanceCI.ppEnabledLayerNames = validationsLayers;
	}

	VK_CHECK(vkCreateInstance(&instanceCI, nullptr, instance));

	// Free allocations
	free(pSDLExtensionsArray);
	free(pExtensionPropertiesArray);
}

static void leSelectPhysicalDevice(const LePhysicalDeviceSelectInfo* pSelectInfo, VkPhysicalDevice* pPhysicalDevice, QueueFamilyIndices* pIndices, SwapchainSupportDetails* pSwapchainDetails)
{
	// Get supported physical devices
	uint32_t physicalDevicesCount;
	vkEnumeratePhysicalDevices(pSelectInfo->instance, &physicalDevicesCount, nullptr);

	assert(physicalDevicesCount > 0 && "no supported gpus found");

	VkPhysicalDevice* pPhysicalDevicesArray = (VkPhysicalDevice*)malloc(physicalDevicesCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(pSelectInfo->instance, &physicalDevicesCount, pPhysicalDevicesArray);

	assert(pPhysicalDevicesArray != NULL && "heap allocation failed");

	bool hasPhysicalDeviceSuitable = false;
	for (uint32_t i = 0; i < physicalDevicesCount; ++i)
	{
		// Get physical device queue families
		uint32_t queueFamilyCount{};
		vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevicesArray[i], &queueFamilyCount, nullptr);

		if (queueFamilyCount == 0)
		{
			continue;
		}

		VkQueueFamilyProperties* pQueueFamiliesArray = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevicesArray[i], &queueFamilyCount, pQueueFamiliesArray);

		assert(pQueueFamiliesArray != NULL && "heap allocation failed");

		pIndices->graphicsFamily = UINT32_MAX;
		pIndices->presentFamily = UINT32_MAX;
		for (uint32_t j = 0; j < queueFamilyCount; ++j)
		{
			uint32_t presentFamily = UINT32_MAX;
			uint32_t graphicsFamily = UINT32_MAX;

			uint32_t presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevicesArray[i], j, pSelectInfo->surface, &presentSupport);

			if (presentSupport)
			{
				presentFamily = j;
			}

			if (pQueueFamiliesArray[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsFamily = j;
			}

			if (presentSupport && graphicsFamily != UINT32_MAX)
			{
				pIndices->graphicsFamily = presentFamily;
				pIndices->presentFamily = graphicsFamily;
				break;
			}
		}

		if (pIndices->graphicsFamily == UINT32_MAX)
		{
			free(pQueueFamiliesArray);
			continue;
		}

		// Get swapchain support ( capabilities, formats, presentModes ) 
		SwapchainSupportDetails details{};
		
		// Swapchain capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevicesArray[i], pSelectInfo->surface, &details.capabilities);

		// Swapchain formats
		uint32_t formatsCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevicesArray[i], pSelectInfo->surface, &formatsCount, nullptr);

		if (formatsCount == 0)
		{
			free(pQueueFamiliesArray);
			continue;
		}

		VkSurfaceFormatKHR* pFormatsArray = (VkSurfaceFormatKHR*)malloc(formatsCount * sizeof(VkSurfaceFormatKHR));
		assert(pFormatsArray != NULL && "heap allocation failed");
		vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevicesArray[i], pSelectInfo->surface, &formatsCount, pFormatsArray);
		details.formats = pFormatsArray;

		// Swapchain presentModes
		uint32_t presentModesCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevicesArray[i], pSelectInfo->surface, &presentModesCount, nullptr);

		if (presentModesCount == 0)
		{
			free(pQueueFamiliesArray);
			free(pFormatsArray);
			continue;
		}

		VkPresentModeKHR* pPresentModesArray = (VkPresentModeKHR*)malloc(presentModesCount * sizeof(VkPresentModeKHR));
		assert(pPresentModesArray != NULL && "heap allocation failed");
		vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevicesArray[i], pSelectInfo->surface, &presentModesCount, pPresentModesArray);
		details.presentModes = pPresentModesArray;

		// Physical Device selected
		hasPhysicalDeviceSuitable = true;
		*pPhysicalDevice = pPhysicalDevicesArray[i];
		*pSwapchainDetails = details;

		// Free
		free(pQueueFamiliesArray);
		break;
	}

	assert(hasPhysicalDeviceSuitable && "No suitable physical device");

	// Free allocations
	free(pPhysicalDevicesArray);
}

static void leCreateDevice(const LeDeviceCreateInfo* pCreateInfo, VkDevice* pDevice)
{
	bool isExclusive = pCreateInfo->indices.graphicsFamily == pCreateInfo->indices.presentFamily;

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo graphicsQueueCI{};
	graphicsQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCI.queueFamilyIndex = pCreateInfo->indices.graphicsFamily;
	graphicsQueueCI.pQueuePriorities = &queuePriority;
	graphicsQueueCI.queueCount = 1;

	VkDeviceQueueCreateInfo presentQueueCI{};
	if (!isExclusive)
	{
		presentQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCI.queueFamilyIndex = pCreateInfo->indices.graphicsFamily;
		presentQueueCI.pQueuePriorities = &queuePriority;
		presentQueueCI.queueCount = 1;
	}

	VkDeviceQueueCreateInfo queuesCI[2] = { graphicsQueueCI, presentQueueCI };

	VkPhysicalDeviceFeatures features{};

	const char* deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCI{};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.queueCreateInfoCount = isExclusive ? 1 : 2;
	deviceCI.pQueueCreateInfos = isExclusive ? &graphicsQueueCI : queuesCI;
	deviceCI.enabledExtensionCount = 1;
	deviceCI.ppEnabledExtensionNames = &deviceExtensions;
	deviceCI.pEnabledFeatures = NULL;

	VK_CHECK(vkCreateDevice(pCreateInfo->physicalDevice, &deviceCI, nullptr, pDevice));
}

static void leCreateAllocator(const LeAllocatorCreateInfo* pCreateInfo, VmaAllocator* pAllocator)
{
	VmaAllocatorCreateInfo allocatorCI{};
	allocatorCI.instance = pCreateInfo->instance;
	allocatorCI.physicalDevice = pCreateInfo->physicalDevice;
	allocatorCI.device = pCreateInfo->device;

	VK_CHECK(vmaCreateAllocator(&allocatorCI, pAllocator));
}

static void leCreateSwapchain(const LeWindow* pWindow, const LeSwapchainCreateInfo* pCreateInfo, VkSwapchainKHR* pSwapchain, VkSurfaceFormatKHR* pFormat)
{
	uint32_t imageCount = pCreateInfo->device.details.capabilities.minImageCount + 1;

	if (pCreateInfo->device.details.capabilities.maxImageCount > 0 && imageCount > pCreateInfo->device.details.capabilities.maxImageCount)
	{
		imageCount = pCreateInfo->device.details.capabilities.maxImageCount;
	}

	// Choose swapchain surface format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pCreateInfo->device.physicalDevice, pCreateInfo->device.surface, &formatCount, nullptr);
	VkSurfaceFormatKHR selectedSurfaceFormat{};
	for (uint32_t i = 0; i < formatCount; ++i)
	{
		if (pCreateInfo->device.details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && pCreateInfo->device.details.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			selectedSurfaceFormat = pCreateInfo->device.details.formats[i];
			break;
		}
	}

	if (selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
	{
		selectedSurfaceFormat = pCreateInfo->device.details.formats[0];
	}

	// Choose swapchain present mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pCreateInfo->device.physicalDevice, pCreateInfo->device.surface, &presentModeCount, nullptr);
	VkPresentModeKHR selectedPresentMode{};
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (pCreateInfo->device.details.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			selectedPresentMode = pCreateInfo->device.details.presentModes[i];
			break;
		}
	}

	if (selectedPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
	{
		selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	// Choose swapchain extent
	VkExtent2D selectedExtent{};
	if (pCreateInfo->device.details.capabilities.currentExtent.width != UINT32_MAX)
	{
		selectedExtent = pCreateInfo->device.details.capabilities.currentExtent;
	}
	else
	{
		int width, height;
		SDL_Vulkan_GetDrawableSize(pWindow->handle, &width, &height);
		selectedExtent = { (uint32_t)width, (uint32_t)height };
		SDL_clamp(selectedExtent.width, pCreateInfo->device.details.capabilities.minImageExtent.width, pCreateInfo->device.details.capabilities.maxImageExtent.width);
		SDL_clamp(selectedExtent.height, pCreateInfo->device.details.capabilities.minImageExtent.height, pCreateInfo->device.details.capabilities.maxImageExtent.height);
	}

	// Create swapchain
	VkSwapchainCreateInfoKHR swapchainCI{};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.surface = pCreateInfo->device.surface;
	swapchainCI.minImageCount = imageCount;
	swapchainCI.imageFormat = selectedSurfaceFormat.format;
	swapchainCI.imageColorSpace = selectedSurfaceFormat.colorSpace;
	swapchainCI.imageExtent = selectedExtent;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = pCreateInfo->device.details.capabilities.currentTransform;
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCI.presentMode = selectedPresentMode;
	swapchainCI.oldSwapchain = VK_NULL_HANDLE;

	bool isExclusive = pCreateInfo->device.queueIndices.graphicsFamily == pCreateInfo->device.queueIndices.presentFamily;

	if (isExclusive)
	{
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		uint32_t queueFamilyIndices[] = { pCreateInfo->device.queueIndices.graphicsFamily, pCreateInfo->device.queueIndices.presentFamily };
		swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCI.queueFamilyIndexCount = 2;
		swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
	}

	VK_CHECK(vkCreateSwapchainKHR(pCreateInfo->device.handle, &swapchainCI, nullptr, pSwapchain));

	*pFormat = selectedSurfaceFormat;
}

static void leCreateSwapchainImages(const LeSwapchainImagesCreateInfo* pCreateInfo, VkImage** ppImages, uint32_t* pImageCount)
{
	vkGetSwapchainImagesKHR(pCreateInfo->device, pCreateInfo->swapchain, pImageCount, nullptr);
	*ppImages = (VkImage*)malloc(*pImageCount * sizeof(VkImage));
	assert(*ppImages != NULL && "heap allocation failed");
	vkGetSwapchainImagesKHR(pCreateInfo->device, pCreateInfo->swapchain, pImageCount, *ppImages);
}

static void leCreateSwapchainImageViews(const LeSwapchainImageViewCreateInfo* pCreateInfo, VkImageView** ppImageViews)
{
	*ppImageViews = (VkImageView*)malloc(pCreateInfo->imageCount * sizeof(VkImageView));
	assert(*ppImageViews != NULL && "heap allocation failed");

	for (uint32_t i = 0; i < pCreateInfo->imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = pCreateInfo->pImages[i];
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = pCreateInfo->format.format;
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(pCreateInfo->device, &imageViewCI, nullptr, &(*ppImageViews)[i]));
	}
}


