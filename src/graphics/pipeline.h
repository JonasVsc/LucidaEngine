#pragma once

#include "pipeline_builder.h"

// lib
#include <vulkan/vulkan.h>

// std
#include <vector>

class Device;

class Pipeline {
public:

	Pipeline(Device& device);

	~Pipeline();

	void bind(VkCommandBuffer cmd);

	VkPipeline m_handle;
private:

	Device& m_device;
};