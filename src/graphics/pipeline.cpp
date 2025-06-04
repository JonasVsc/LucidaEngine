#include "pipeline.h"

#include "core/log.h"
#include "graphics/device.h"


Pipeline::Pipeline(Device& device)
	: m_device{device}
{
	jinfo("pipeline constructor");
}

Pipeline::~Pipeline()
{
	jinfo("pipeline destructor");
	vkDestroyPipeline(m_device.get_handle(), m_handle, nullptr);
}

void Pipeline::bind(VkCommandBuffer cmd)
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle);
}