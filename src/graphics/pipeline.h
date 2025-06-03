#pragma once

#include <vulkan/vulkan.h>


struct PipelineBuilder {
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	
	VkPipelineVertexInputStateCreateInfo vertexInputState;

	VkPipelineRasterizationStateCreateInfo rasterizationState;
	
	VkPipelineViewportStateCreateInfo viewportState;

	VkPipelineDynamicStateCreateInfo dynamicState;

};

class Pipeline {
public:

private:

};