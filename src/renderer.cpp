#include "lucida/renderer.h"
#include "lucida/window.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	VkPresentModeKHR* presentModes;
};

// Create vulkan instance
static void leCreateInstance(const LeWindow* pWindow, VkInstance* pInstance);
static void leSelectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* pPhysicalDevice, QueueFamilyIndices* pIndices);
static void leCreateDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices, VkDevice* pDevice);
static void leCreateAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator* pAllocator);

LeResult leRendererInit(const LeRendererCreateInfo* pCreateInfo, LeRenderer* pRenderer)
{
	leCreateInstance(
		pCreateInfo->pWindow, 
		&pRenderer->instance);
	
	SDL_Vulkan_CreateSurface(
		pCreateInfo->pWindow->handle,
		pRenderer->instance, 
		&pRenderer->surface);
	
	leSelectPhysicalDevice(
		pRenderer->instance,
		pRenderer->surface,
		&pRenderer->physicalDevice,
		&pRenderer->indices);
	
	leCreateDevice(
		pRenderer->physicalDevice,
		pRenderer->indices, 
		&pRenderer->device);

	vkGetDeviceQueue(
		pRenderer->device, 
		pRenderer->indices.graphicsFamily,
		0, &pRenderer->graphicsQueue);

	vkGetDeviceQueue
	(pRenderer->device, 
		pRenderer->indices.presentFamily, 
		0, &pRenderer->presentQueue);

	leCreateAllocator(
		pRenderer->instance,
		pRenderer->physicalDevice,
		pRenderer->device,
		&pRenderer->allocator);

	return LE_SUCCESS;
}

void leRendererShutdown(LeRenderer* pRenderer)
{
	if (!pRenderer)
	{
		return;
	}


	vmaDestroyAllocator(pRenderer->allocator);
	vkDestroyDevice(pRenderer->device, nullptr);
	vkDestroySurfaceKHR(pRenderer->instance, pRenderer->surface, nullptr);
	vkDestroyInstance(pRenderer->instance, nullptr);
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

static void leSelectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice* pPhysicalDevice, QueueFamilyIndices* pIndices)
{
	// Get supported physical devices
	uint32_t physicalDevicesCount;
	vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr);

	assert(physicalDevicesCount > 0 && "no supported gpus found");

	VkPhysicalDevice* pPhysicalDevicesArray = (VkPhysicalDevice*)malloc(physicalDevicesCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, pPhysicalDevicesArray);

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
			vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevicesArray[i], j, surface, &presentSupport);

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
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevicesArray[i], surface, &details.capabilities);

		// Swapchain formats
		uint32_t formatsCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevicesArray[i], surface, &formatsCount, nullptr);

		if (formatsCount == 0)
		{
			free(pQueueFamiliesArray);
			continue;
		}

		VkSurfaceFormatKHR* pFormatsArray = (VkSurfaceFormatKHR*)malloc(formatsCount * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevicesArray[i], surface, &formatsCount, pFormatsArray);

		assert(pFormatsArray != NULL && "heap allocation failed");
		details.formats = pFormatsArray;

		// Swapchain presentModes
		uint32_t presentModesCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevicesArray[i], surface, &presentModesCount, nullptr);

		if (presentModesCount == 0)
		{
			free(pQueueFamiliesArray);
			free(pFormatsArray);
			continue;
		}

		VkPresentModeKHR* pPresentModesArray = (VkPresentModeKHR*)malloc(presentModesCount * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevicesArray[i], surface, &presentModesCount, nullptr);

		assert(pPresentModesArray != NULL && "heap allocation failed");
		details.presentModes = pPresentModesArray;

		// Physical Device selected
		hasPhysicalDeviceSuitable = true;
		*pPhysicalDevice = pPhysicalDevicesArray[i];

		// Free
		free(pQueueFamiliesArray);
		free(pFormatsArray);
		free(pPresentModesArray);
		break;
	}

	assert(hasPhysicalDeviceSuitable && "No suitable physical device");

	// Free allocations
	free(pPhysicalDevicesArray);
}

static void leCreateDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices, VkDevice* pDevice)
{
	bool isExclusive = indices.graphicsFamily == indices.presentFamily;

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo graphicsQueueCI{};
	graphicsQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCI.queueFamilyIndex = indices.graphicsFamily;
	graphicsQueueCI.pQueuePriorities = &queuePriority;
	graphicsQueueCI.queueCount = 1;

	VkDeviceQueueCreateInfo presentQueueCI{};
	if (!isExclusive)
	{
		presentQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCI.queueFamilyIndex = indices.graphicsFamily;
		presentQueueCI.pQueuePriorities = &queuePriority;
		presentQueueCI.queueCount = 1;
	}

	VkDeviceQueueCreateInfo queuesCI[2] = { graphicsQueueCI, presentQueueCI };

	VkPhysicalDeviceFeatures features{};

	VkDeviceCreateInfo deviceCI{};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.queueCreateInfoCount = isExclusive ? 1 : 2;
	deviceCI.pQueueCreateInfos = isExclusive ? &graphicsQueueCI : queuesCI;
	deviceCI.enabledExtensionCount = 0;
	deviceCI.ppEnabledExtensionNames = NULL;
	deviceCI.pEnabledFeatures = NULL;

	VK_CHECK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, pDevice));
}

static void leCreateAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator* pAllocator)
{
	VmaAllocatorCreateInfo allocatorCI{};
	allocatorCI.instance = instance;
	allocatorCI.physicalDevice = physicalDevice;
	allocatorCI.device = device;

	VK_CHECK(vmaCreateAllocator(&allocatorCI, pAllocator));
}

