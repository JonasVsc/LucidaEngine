#include "renderer.h"
#include "window.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

void leRendererCreate(LeWindow& window, LeRenderer& renderer)
{
	// VkInstance
	{
		// SDL Extensions
		uint32_t extensionCount;
		SDL_Vulkan_GetInstanceExtensions(window.handle, &extensionCount, nullptr);
		std::vector<const char*> sdlExtensions(extensionCount);
		SDL_Vulkan_GetInstanceExtensions(window.handle, &extensionCount, sdlExtensions.data());

		// Instance Extensions
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> instanceExtentions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtentions.data());

#ifdef DEBUG
		printf("\nRequired SDL Extensions:\n");
		for (const auto& sdl : sdlExtensions)
		{
			printf("- %s\n", sdl);
		}

		printf("\nSupported Instance Extensions:\n");
		for (const auto& ext : instanceExtentions)
		{
			printf("- %s\n", ext.extensionName);
		}
#endif

		for (const auto& sdlExt : sdlExtensions)
		{
			bool hasSupport = false;
			for (const auto& ext : instanceExtentions)
			{
				if (!strcmp(sdlExt, ext.extensionName))
				{
					hasSupport = true;
					break;
				}
			}
			assert(hasSupport && "unsupported extension: ");
		}

		VkApplicationInfo appInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0)
		};

		VkInstanceCreateInfo instanceCI{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = static_cast<uint32_t>(sdlExtensions.size()),
			.ppEnabledExtensionNames = sdlExtensions.data()
		};

		if (ENABLE_VALIDATION_LAYERS)
		{
			const char* validationsLayers[] = { "VK_LAYER_KHRONOS_validation" };
			instanceCI.enabledLayerCount = 1;
			instanceCI.ppEnabledLayerNames = validationsLayers;
		}

		VK_CHECK(vkCreateInstance(&instanceCI, nullptr, &renderer.instance));
	} // End VkInstance

	// Create VkSurfaceKHR
	SDL_Vulkan_CreateSurface(window.handle, renderer.instance, &renderer.surface);


	// Select VkPhysicalDevice
	{
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(renderer.instance, &physicalDeviceCount, nullptr);
		assert(physicalDeviceCount > 0 && "no suitable gpu found");
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(renderer.instance, &physicalDeviceCount, physicalDevices.data());

		bool isSuitable = false;
		for (const auto& physicalDevice : physicalDevices)
		{
			uint32_t queueFamilyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
			if (queueFamilyCount == 0)
				continue;
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

			renderer.presentFamily = UINT32_MAX;
			renderer.graphicsFamily = UINT32_MAX;
			for (uint32_t index = 0; index < queueFamilyCount; ++index)
			{
				uint32_t presentIndex = UINT32_MAX;
				uint32_t graphicsIndex = UINT32_MAX;
				uint32_t presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, renderer.surface, &presentSupport);

				if (presentSupport)
					presentIndex = index;
				if (queueFamilies[index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					graphicsIndex = index;

				if (presentIndex != UINT32_MAX && graphicsIndex != UINT32_MAX)
				{
					renderer.graphicsFamily = graphicsIndex;
					renderer.presentFamily = presentIndex;
					break;
				}
			}

			if (renderer.graphicsFamily == UINT32_MAX)
				continue;


			// Surface capabilities
			VkSurfaceCapabilitiesKHR capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, renderer.surface, &capabilities);

			// Surface formats
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, renderer.surface, &formatCount, nullptr);
			if (formatCount == 0)
				continue;
			std::vector<VkSurfaceFormatKHR> formats(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, renderer.surface, &formatCount, formats.data());

			// Surface presentModes
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, renderer.surface, &presentModeCount, nullptr);
			if (presentModeCount == 0)
				continue;
			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, renderer.surface, &presentModeCount, presentModes.data());

			renderer.swapchainDetails.capabilities = capabilities;
			renderer.swapchainDetails.formats = formats;
			renderer.swapchainDetails.presentModes = presentModes;
			renderer.physicalDevice = physicalDevice;
			isSuitable = true;
			break;
		}

#ifdef DEBUG
		printf("\nSelected Physical Device Info:\n");
		printf("- Graphics Family: %d\n", renderer.graphicsFamily);
		printf("- Present Family: %d\n", renderer.presentFamily);
		printf("- Swapchain Details:\n");
		printf("\t- formats: %zd\n", renderer.swapchainDetails.formats.size());
		printf("\t- presentModes: %zd\n", renderer.swapchainDetails.presentModes.size());
#endif

	} // End VkPhysicalDevice


	// Create VkDevice
	{
		bool isExclusive = renderer.graphicsFamily == renderer.presentFamily;

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo graphicsQueueCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = renderer.graphicsFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};


		VkDeviceQueueCreateInfo presentQueueCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = renderer.graphicsFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkDeviceQueueCreateInfo queuesCI[2] = { graphicsQueueCI, presentQueueCI };

		VkPhysicalDeviceFeatures features{};

		const char* deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = isExclusive ? (uint32_t)1 : (uint32_t)2,
			.pQueueCreateInfos = isExclusive ? &graphicsQueueCI : queuesCI,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = &deviceExtensions,
			.pEnabledFeatures = NULL
		};

		VK_CHECK(vkCreateDevice(renderer.physicalDevice, &deviceCI, nullptr, &renderer.device));

		vkGetDeviceQueue(renderer.device, renderer.graphicsFamily, 0, &renderer.graphicsQueue);
		vkGetDeviceQueue(renderer.device, renderer.presentFamily, 0, &renderer.presentQueue);

	} // End VkDevice
	 
	// Create VmaAllocator
	{
		VmaAllocatorCreateInfo allocatorCI{
			.physicalDevice = renderer.physicalDevice,
			.device = renderer.device,
			.instance = renderer.instance
		};

		VK_CHECK(vmaCreateAllocator(&allocatorCI, &renderer.allocator));
	} // End VmaAllocator


	// Create VkSwapchainKHR and Images, ImageViews
	{
		uint32_t imageCount = renderer.swapchainDetails.capabilities.minImageCount + 1;
		if (renderer.swapchainDetails.capabilities.maxImageCount > 0 && imageCount > renderer.swapchainDetails.capabilities.maxImageCount)
			imageCount = renderer.swapchainDetails.capabilities.maxImageCount;

#ifdef DEBUG

		printf("\nSwapchain:\n");
		printf("- Images: %d\n", imageCount);

#endif

		// Choose surface format
		VkSurfaceFormatKHR selectedSurfaceFormat{};
		for (const auto& format : renderer.swapchainDetails.formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				selectedSurfaceFormat = format;
				break;
			}
		}

		if (selectedSurfaceFormat.format == VK_FORMAT_UNDEFINED)
			selectedSurfaceFormat = renderer.swapchainDetails.formats[0];


		// Choose present mode
		VkPresentModeKHR selectedPresentMode{};
		for (const auto& presentMode : renderer.swapchainDetails.presentModes)
		{
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				selectedPresentMode = presentMode;
				break;
			}
		}

		if (selectedPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// Choose extent
		VkExtent2D selectedExtent{};
		if (renderer.swapchainDetails.capabilities.currentExtent.width != UINT32_MAX)
		{
			selectedExtent = renderer.swapchainDetails.capabilities.currentExtent;
		}
		else
		{
			int width, height;
			SDL_Vulkan_GetDrawableSize(window.handle, &width, &height);
			selectedExtent = { (uint32_t)width, (uint32_t)height };
			SDL_clamp(selectedExtent.width, renderer.swapchainDetails.capabilities.minImageExtent.width, renderer.swapchainDetails.capabilities.maxImageExtent.width);
			SDL_clamp(selectedExtent.height, renderer.swapchainDetails.capabilities.minImageExtent.height, renderer.swapchainDetails.capabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR swapchainCI{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = renderer.surface,
			.minImageCount = imageCount,
			.imageFormat = selectedSurfaceFormat.format,
			.imageColorSpace = selectedSurfaceFormat.colorSpace,
			.imageExtent = selectedExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = renderer.swapchainDetails.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = selectedPresentMode,
			.oldSwapchain = VK_NULL_HANDLE 
		};

		bool isExclusive = renderer.graphicsFamily == renderer.presentFamily;

		if (isExclusive)
		{
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		else
		{
			uint32_t queueFamilyIndices[] = { renderer.graphicsFamily, renderer.presentFamily };
			swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCI.queueFamilyIndexCount = 2;
			swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
		}

		VK_CHECK(vkCreateSwapchainKHR(renderer.device, &swapchainCI, nullptr, &renderer.swapchain));

		renderer.surfaceFormat = selectedSurfaceFormat;
		renderer.surfaceExtent = selectedExtent;

		// Images
		vkGetSwapchainImagesKHR(renderer.device, renderer.swapchain, &imageCount, nullptr);
		renderer.swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(renderer.device, renderer.swapchain, &imageCount, renderer.swapchainImages.data());

		// Image views
		renderer.swapchainImageViews.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo imageViewCI{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = renderer.swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = renderer.surfaceFormat.format,
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

			VK_CHECK(vkCreateImageView(renderer.device, &imageViewCI, nullptr, &renderer.swapchainImageViews[i]));
		}

	} // End VkSwapchainKHR and Images, ImageViews

	// Create VkRenderPass
	{
		VkAttachmentDescription colorAttachment{
			.format = renderer.surfaceFormat.format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef
		};

		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkAttachmentDescription attachments[] = { colorAttachment };

		VkRenderPassCreateInfo renderPassCI{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency
		};

		VK_CHECK(vkCreateRenderPass(renderer.device, &renderPassCI, nullptr, &renderer.renderPass));

	} // End VkRenderPass


	// Create Framebuffers
	{
		renderer.framebuffers.resize(renderer.swapchainImageViews.size());

#ifdef DEBUG
		printf("\nFramebuffers:\n");
		printf("- Count: %d\n", (int)renderer.framebuffers.size());
#endif 


		for (uint32_t i = 0; i < renderer.swapchainImageViews.size(); ++i)
		{
			VkImageView attachments[] = { renderer.swapchainImageViews[i]};

			VkFramebufferCreateInfo framebufferCI{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = renderer.renderPass,
				.attachmentCount = 1,
				.pAttachments = attachments,
				.width = renderer.surfaceExtent.width,
				.height = renderer.surfaceExtent.height,
				.layers = 1
			};

			VK_CHECK(vkCreateFramebuffer(renderer.device, &framebufferCI, nullptr, &renderer.framebuffers[i]));
		}

	} // End Framebuffers


	// Create VkCommandPool and VkCommandBuffers
	{
		VkCommandPoolCreateInfo commandPoolCI{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = renderer.graphicsFamily,
		};

		VK_CHECK(vkCreateCommandPool(renderer.device, &commandPoolCI, nullptr, &renderer.commandPool));

		renderer.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo commandBufferCI{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = renderer.commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = (uint32_t)renderer.commandBuffers.size()
		};

		VK_CHECK(vkAllocateCommandBuffers(renderer.device, &commandBufferCI, renderer.commandBuffers.data()));

#ifdef DEBUG
		printf("\nCommand Buffers:\n");
		printf("Count: %d\n", (int)renderer.commandBuffers.size());
#endif
	} // End VkCommandPool amd VkCommandBuffers


	// Create Sync Objects ( VkSemaphore, VkFence )
	{
		renderer.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderer.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderer.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

#ifdef DEBUG
		printf("\nSync Objects:\n");
		printf("- Semaphores\n");
		printf("\t- ImageAvailableSemaphore: %zd\n", renderer.imageAvailableSemaphores.size());
		printf("\t- RenderFinishedSemaphores: %zd\n", renderer.renderFinishedSemaphores.size());
		printf("- Fences\n");
		printf("\t- InFlightFences: %zd\n", renderer.inFlightFences.size());
#endif 


		VkSemaphoreCreateInfo semaphoreCI{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		VkFenceCreateInfo fenceCI{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK(vkCreateSemaphore(renderer.device, &semaphoreCI, nullptr, &renderer.imageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(renderer.device, &semaphoreCI, nullptr, &renderer.renderFinishedSemaphores[i]));
			VK_CHECK(vkCreateFence(renderer.device, &fenceCI, nullptr, &renderer.inFlightFences[i]));
		}
	} // End Sync Objects
}

void leRendererDestroy(LeRenderer& renderer)
{
	vkDeviceWaitIdle(renderer.device);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(renderer.device, renderer.imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(renderer.device, renderer.renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(renderer.device, renderer.inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(renderer.device, renderer.commandPool, nullptr);

	for (const auto framebuffer : renderer.framebuffers)
	{
		vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(renderer.device, renderer.renderPass, nullptr);

	for (const auto imageView : renderer.swapchainImageViews)
	{
		vkDestroyImageView(renderer.device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(renderer.device, renderer.swapchain, nullptr);
	vmaDestroyAllocator(renderer.allocator);
	vkDestroyDevice(renderer.device, nullptr);
	vkDestroySurfaceKHR(renderer.instance, renderer.surface, nullptr);
	vkDestroyInstance(renderer.instance, nullptr);
}
