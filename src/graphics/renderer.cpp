#include "renderer.h"

// core
#include "core/log.h"

Renderer::Renderer(Config& config, Window& window)
	: m_config{config}
	, m_window{window}
{
	jinfo("renderer constructor");
	create_pipeline_layout();
	create_render_pass();
}

Renderer::~Renderer()
{
	jinfo("renderer destructor");
	vkDestroyPipelineLayout(m_device.get_handle(), m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_device.get_handle(), m_render_pass, nullptr);
}

void Renderer::create_render_pass()
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = m_swapchain.get_image_format();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> attachments = { color_attachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(m_device.get_handle(), &renderPassInfo, nullptr, &m_render_pass));
}

void Renderer::create_pipeline_layout()
{
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	VK_CHECK(vkCreatePipelineLayout(m_device.get_handle(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout));
}
