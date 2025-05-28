#include "renderer.h"

// lib
#include "debug/log.h"

#ifdef DEBUG

static void print_physical_device_properties(VkPhysicalDeviceProperties* pPhysicalDeviceProperties)
{
	fmt::println("{}:", pPhysicalDeviceProperties->deviceName);
	fmt::println("\t- maxPushConstantSize: {}", pPhysicalDeviceProperties->limits.maxPushConstantsSize);
	fmt::println("\t- memoryMapAlignment: {}", pPhysicalDeviceProperties->limits.minMemoryMapAlignment);
}

#endif // DEBUG

Renderer::Renderer()
{

	jinfo("renderer constructor");
	
	// VkInstance
	{ 
		VkApplicationInfo app_info = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = nullptr,
			.applicationVersion = 0,
			.pEngineName = nullptr,
			.engineVersion = 0,
			.apiVersion = 0
		};

		VkInstanceCreateInfo instance_create_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = 0,
			.ppEnabledExtensionNames = nullptr
		};

		VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &m_instance));
	}

	// VkPhysicalDevice
	{
		uint32_t count_physical_devices;
		vkEnumeratePhysicalDevices(m_instance, &count_physical_devices, nullptr);

		std::vector<VkPhysicalDevice> physical_devices(count_physical_devices);
		vkEnumeratePhysicalDevices(m_instance, &count_physical_devices, physical_devices.data());


#ifdef DEBUG
		for (auto phys : physical_devices)
		{
			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(phys, &prop);
			print_physical_device_properties(&prop);
		}
#endif // DEBUG

		// TODO: Algorithm: Select best physical device
		m_physical_device = physical_devices[0];
	}

	// Device
	{

	}
}

Renderer::~Renderer()
{
	destroy_renderer();
}

void Renderer::destroy_renderer()
{
	jinfo("renderer destructor");
}