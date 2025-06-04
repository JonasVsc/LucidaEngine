#pragma once

// lib
#include <vulkan/vulkan.h>

// std
#include <vector>
#include <string>

class Device;
class Pipeline;

class PipelineBuilder {
public:

	static PipelineBuilder create(VkPipelineLayout pipeline_layout, VkRenderPass render_pass);

	PipelineBuilder& add_shader_stage(VkShaderModule module, VkShaderStageFlagBits stage);

	PipelineBuilder& set_input_assembly(VkPrimitiveTopology topology);

	PipelineBuilder& set_rasterizer();

	PipelineBuilder& set_multisampling();

	PipelineBuilder& add_color_blend_attachment();

	PipelineBuilder& set_color_blend();

	PipelineBuilder& set_depth_stencil();

	PipelineBuilder& set_dynamic_states(VkDynamicState state);

	PipelineBuilder& set_debug_name(const std::string& name);

	Pipeline build(Device& device);

public:

	std::string m_debug_name;

	VkPipelineLayoutCreateInfo m_pipeline_layout_create_info{};

	std::vector<VkVertexInputBindingDescription> m_binding_descriptions;

	std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;

	std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages{};

	VkPipelineVertexInputStateCreateInfo m_vertex_input{};

	VkPipelineInputAssemblyStateCreateInfo m_input_assembly{};

	VkPipelineViewportStateCreateInfo m_viewport{};

	VkPipelineRasterizationStateCreateInfo m_rasterizer{};

	VkPipelineMultisampleStateCreateInfo m_multisampling{};

	std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachments;

	VkPipelineColorBlendStateCreateInfo m_color_blend{};

	VkPipelineDepthStencilStateCreateInfo m_depth_stencil{};

	std::vector<VkDynamicState> m_dynamic_states{};

	VkPipelineDynamicStateCreateInfo m_dynamic{};

	VkPipelineLayout m_pipeline_layout{};

	VkRenderPass m_render_pass{};

	uint32_t m_subpass{};

private:

	PipelineBuilder();

};

