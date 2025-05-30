#include "renderer.h"

// lucida
#include "debug/log.h"
#include "window/window.h"

// std
#include <unordered_set>
#include <map>
#include <set>

#ifdef DEBUG

static void print_physical_device_properties(VkPhysicalDeviceProperties* pPhysicalDeviceProperties)
{
	jdebug("Selected Physical Device");
	fmt::println("{}", pPhysicalDeviceProperties->deviceName);
	fmt::println(" - maxPushConstantSize: {}", pPhysicalDeviceProperties->limits.maxPushConstantsSize);
	fmt::println(" - memoryMapAlignment: {}", pPhysicalDeviceProperties->limits.minMemoryMapAlignment);
}

#endif // DEBUG

static int rate_physical_device_suitability(VkPhysicalDevice physical_device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	int score{};

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	score += properties.limits.maxImageDimension2D;

	return score;
}

QueueFamilyIndices Renderer::find_queue_families(VkPhysicalDevice physical_device)
{
	QueueFamilyIndices indices;

	uint32_t count_queue_family;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count_queue_family, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(count_queue_family);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count_queue_family, queue_families.data());

	int i = 0;
	for (const auto& queue_family : queue_families)
	{
		uint32_t support_present = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, m_context.surface, &support_present);
		
		if (support_present)
		{
			indices.present_family = i;
		}

		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_family = i;
		}

		if (indices.is_complete())
		{
			break;
		}

		i++;
	}
	return indices;
}

bool Renderer::is_physical_device_suitable(VkPhysicalDevice physical_device)
{
	QueueFamilyIndices indices = find_queue_families(physical_device);
	if (indices.is_exclusive())
		jinfo("SHARING_MODE: EXCLUSIVE");
	else
		jinfo("SHARING_MODE: CONCURRENT");
	return indices.is_complete();
}

Renderer::Renderer(Window& window, LucidaConfig& lc)
	: m_window{window}
	, m_config{lc}
{
	jinfo("renderer constructor");
	create_instance();
	SDL_Vulkan_CreateSurface(m_window.w_sdl(), m_context.instance, &m_context.surface);
	select_physical_device();
	create_device();
}

Renderer::~Renderer()
{
	jinfo("renderer destructor");
	vkDestroyDevice(m_context.device, nullptr);
	vkDestroySurfaceKHR(m_context.instance, m_context.surface, nullptr);
	vkDestroyInstance(m_context.instance, nullptr);
}

void Renderer::create_instance()
{
	// Convert config layers string to c string style
	std::vector<std::string> layers = m_config.get_layers();
	std::vector<const char*> cLayers;
	for (const auto& layer : layers)
	{
		cLayers.push_back(layer.c_str());
	}

	// Convert config extensions string to c string style
	std::vector<std::string> extensions = m_config.get_extensions();
	std::vector<const char*> cExtensions;
	for (const auto& ext : extensions)
	{
		cExtensions.push_back(ext.c_str());
	}

	// Available Layers
	uint32_t count_layers;
	vkEnumerateInstanceLayerProperties(&count_layers, nullptr);
	std::vector<VkLayerProperties> available_layers(count_layers);
	vkEnumerateInstanceLayerProperties(&count_layers, available_layers.data());

	// Requested Layers
	std::vector<const char*> unsupported_layers;
	std::vector<const char*> supported_layers;
	for (const auto& lay : cLayers)
	{
		bool unsupported = true;
		for (const auto& available_lay : available_layers)
		{
			if (!strcmp(available_lay.layerName, lay))
			{
				supported_layers.push_back(lay);
				unsupported = false;
			}
		}

		if (unsupported)
		{
			unsupported_layers.push_back(lay);
		}
	}

	if (!unsupported_layers.empty())
	{
		jerr("layers unsupported:");
		for (const auto& lay : unsupported_layers)
		{
			fmt::println("- {}", lay);
		}
		throw std::runtime_error("instance doesn't support requested layers");
	}

	// Available Extensions
	uint32_t count_extensions;
	vkEnumerateInstanceExtensionProperties(nullptr, &count_extensions, nullptr);
	std::vector<VkExtensionProperties> available_extensions(count_extensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &count_extensions, available_extensions.data());

	// Required SDL Extensions
	uint32_t count_sdl_extensions;
	SDL_Vulkan_GetInstanceExtensions(m_window.w_sdl(), &count_sdl_extensions, nullptr);
	std::vector<const char*> sdl_extensions(count_sdl_extensions);
	SDL_Vulkan_GetInstanceExtensions(m_window.w_sdl(), &count_sdl_extensions, sdl_extensions.data());

	// Requested/Required Extensions
	std::vector<const char*> required_extensions;
	required_extensions.insert(required_extensions.end(), sdl_extensions.begin(), sdl_extensions.end());
	required_extensions.insert(required_extensions.end(), cExtensions.begin(), cExtensions.end());

	// Check if Instance supports Requested/Required extensions
	std::unordered_set<const char*> extensions_set;
	extensions_set.insert(required_extensions.begin(), required_extensions.end());
	std::vector<const char*> unsupported_extensions;
	std::vector<const char*> supported_extensions;
	for (const auto& ext : extensions_set)
	{
		bool unsupported = true;
		for (const auto& available_ext : available_extensions)
		{
			if (!strcmp(available_ext.extensionName, ext))
			{
				supported_extensions.push_back(ext);
				unsupported = false;
			}
		}

		if (unsupported)
		{
			unsupported_extensions.push_back(ext);
		}
	}

	if (!unsupported_extensions.empty())
	{
		jwarn("Trying to initialize renderer without unsupported extensions:");
		for (const auto& ext : unsupported_extensions)
		{
			fmt::println("- {}", ext);
		}
	}

#ifdef DEBUG
	jdebug("Using layers:");
	for (auto& lay : cLayers)
		fmt::println("- {}", lay);
	jdebug("Using extensions:");
	for (const auto& ext : required_extensions)
		fmt::println("- {}", ext);
#endif

	uint32_t appVersion = VK_MAKE_API_VERSION(
		0 /* VARIANT */, m_config.get_app_version()[0] /* MAJOR */, m_config.get_app_version()[1] /* MINOR */, m_config.get_app_version()[2] /* PATCH */);
	uint32_t apiVersion = VK_MAKE_API_VERSION(
		0 /* VARIANT */, m_config.get_api_version()[0] /* MAJOR */, m_config.get_api_version()[1] /* MINOR */, m_config.get_api_version()[2] /* PATCH */);
	uint32_t engineVersion = VK_MAKE_API_VERSION(
		0 /* VARIANT */, m_config.get_lucida_version()[0] /* MAJOR */, m_config.get_lucida_version()[1] /* MINOR */, m_config.get_lucida_version()[2] /* PATCH */);

	VkApplicationInfo app_info = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = m_config.get_app_name().c_str(),
			.applicationVersion = appVersion,
			.pEngineName = "Lucida",
			.engineVersion = engineVersion,
			.apiVersion = apiVersion
	};

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<uint32_t>(cLayers.size()),
		.ppEnabledLayerNames = cLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(supported_extensions.size()),
		.ppEnabledExtensionNames = supported_extensions.data()
	};

	VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &m_context.instance));
}

void Renderer::select_physical_device()
{
	uint32_t count_physical_devices;
	vkEnumeratePhysicalDevices(m_context.instance, &count_physical_devices, nullptr);
	if (!count_physical_devices)
	{
		throw std::runtime_error("failed to find GPUs with vulkan support");
	}

	std::vector<VkPhysicalDevice> physical_devices(count_physical_devices);
	vkEnumeratePhysicalDevices(m_context.instance, &count_physical_devices, physical_devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& physical_device : physical_devices)
	{
		if (is_physical_device_suitable(physical_device))
		{
			int score = rate_physical_device_suitability(physical_device);
			candidates.insert(std::make_pair(score, physical_device));
		}
	}

	if (candidates.rbegin()->first > 0)
	{
		m_context.physical_device = candidates.rbegin()->second;
		vkGetPhysicalDeviceProperties(candidates.rbegin()->second, &m_context.physical_device_properties);
		jdebug("Selected physical device: {}", m_context.physical_device_properties.deviceName);
		jdebug("Selected physical device score: {}", candidates.rbegin()->first);
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU");
	}
}

void Renderer::create_device()
{
	QueueFamilyIndices indices = find_queue_families(m_context.physical_device);
	
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queue_family,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
		.pQueueCreateInfos = queue_create_infos.data(),
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &device_features
	};

	VK_CHECK(vkCreateDevice(m_context.physical_device, &device_create_info, nullptr, &m_context.device));

	vkGetDeviceQueue(m_context.device, indices.graphics_family.value(), 0, &m_context.graphics_queue);
	vkGetDeviceQueue(m_context.device, indices.present_family.value(), 0, &m_context.present_queue);
}
