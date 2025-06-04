#include "pipeline_builder.h"

#include "core/log.h"
#include "vertex.h"
#include "pipeline.h"
#include "device.h"

PipelineBuilder PipelineBuilder::create(VkPipelineLayout pipeline_layout, VkRenderPass render_pass)
{
	assert(pipeline_layout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no VkPipelineLayout provided in configInfo");
	assert(render_pass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no VkRenderPass provided in configInfo");

	PipelineBuilder pipeline_builder;
	pipeline_builder.m_render_pass = render_pass;
	pipeline_builder.m_pipeline_layout = pipeline_layout;
	return pipeline_builder;
}

PipelineBuilder::PipelineBuilder()
{
	m_debug_name = "default";

	// empty pipeline layout
	m_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	m_attribute_descriptions = Vertex::get_attribute_descriptions();
	m_binding_descriptions = Vertex::get_binding_descriptions();

	m_vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(m_binding_descriptions.size());
	m_vertex_input.pVertexBindingDescriptions = m_binding_descriptions.data();
	m_vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attribute_descriptions.size());
	m_vertex_input.pVertexAttributeDescriptions = m_attribute_descriptions.data();

	// ok
	m_input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_input_assembly.primitiveRestartEnable = VK_FALSE;

	// ok
	m_viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewport.viewportCount = 1;
	m_viewport.pViewports = nullptr;
	m_viewport.scissorCount = 1;
	m_viewport.pScissors = nullptr;

	// ok
	m_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizer.depthClampEnable = VK_FALSE;
	m_rasterizer.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizer.cullMode = VK_CULL_MODE_NONE;
	m_rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_rasterizer.depthBiasEnable = VK_FALSE;
	m_rasterizer.depthBiasConstantFactor = 0.0f;
	m_rasterizer.depthBiasClamp = 0.0f;
	m_rasterizer.depthBiasSlopeFactor = 0.0f;
	m_rasterizer.lineWidth = 1.0f;

	// ok
	m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling.sampleShadingEnable = VK_FALSE;
	m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampling.minSampleShading = 1.0f;
	m_multisampling.pSampleMask = nullptr;
	m_multisampling.alphaToCoverageEnable = VK_FALSE;
	m_multisampling.alphaToOneEnable = VK_FALSE;

	// build required
	m_color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_color_blend.logicOpEnable = VK_FALSE;
	m_color_blend.logicOp = VK_LOGIC_OP_COPY;
	m_color_blend.attachmentCount = 0;
	m_color_blend.pAttachments = nullptr;
	m_color_blend.blendConstants[0] = 0.0f;
	m_color_blend.blendConstants[1] = 0.0f;
	m_color_blend.blendConstants[2] = 0.0f;
	m_color_blend.blendConstants[3] = 0.0f;

	// ok
	m_depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_depth_stencil.depthTestEnable = VK_TRUE;
	m_depth_stencil.depthWriteEnable = VK_TRUE;
	m_depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	m_depth_stencil.depthBoundsTestEnable = VK_FALSE;
	m_depth_stencil.minDepthBounds = 0.0f;
	m_depth_stencil.maxDepthBounds = 1.0f;
	m_depth_stencil.front = {};
	m_depth_stencil.back = {}; 

	// build required
	m_dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
}

PipelineBuilder& PipelineBuilder::add_shader_stage(VkShaderModule module, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shader_stage = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = stage,
		.module = module,
		.pName = "main",
	};

	m_shader_stages.push_back(shader_stage);
	return *this;
}

PipelineBuilder& PipelineBuilder::set_input_assembly(VkPrimitiveTopology topology)
{
	m_input_assembly.topology = topology;
	return *this;
}

PipelineBuilder& PipelineBuilder::set_rasterizer()
{
	return *this;
}

PipelineBuilder& PipelineBuilder::set_multisampling()
{
	return *this;
}

PipelineBuilder& PipelineBuilder::add_color_blend_attachment()
{
	VkPipelineColorBlendAttachmentState attachment = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	m_color_blend_attachments.push_back(attachment);
	return *this;
}

PipelineBuilder& PipelineBuilder::set_color_blend()
{
	return *this;
}

PipelineBuilder& PipelineBuilder::set_depth_stencil()
{
	return *this;
}

PipelineBuilder& PipelineBuilder::set_dynamic_states(VkDynamicState state)
{
	m_dynamic_states.push_back(state);
	return *this;
}

PipelineBuilder& PipelineBuilder::set_debug_name(const std::string& name)
{
	m_debug_name = name;
	return *this;
}

Pipeline PipelineBuilder::build(Device& device)
{
	// apply color blend attachments
	m_color_blend.attachmentCount = static_cast<uint32_t>(m_color_blend_attachments.size());
	m_color_blend.pAttachments = m_color_blend_attachments.data();

	// apply dynamic states
	m_dynamic.dynamicStateCount = static_cast<uint32_t>(m_dynamic_states.size());
	m_dynamic.pDynamicStates = m_dynamic_states.data();

	// create graphics pipeline
	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(m_shader_stages.size()),
		.pStages = m_shader_stages.data(),
		.pVertexInputState = &m_vertex_input,
		.pInputAssemblyState = &m_input_assembly,
		.pViewportState = &m_viewport,
		.pRasterizationState = &m_rasterizer,
		.pMultisampleState = &m_multisampling,
		.pDepthStencilState = &m_depth_stencil,
		.pColorBlendState = &m_color_blend,
		.pDynamicState = &m_dynamic,
		.layout = m_pipeline_layout,
		.renderPass = m_render_pass,
		.subpass = m_subpass
	};

	Pipeline pipeline{device};
	VK_CHECK(vkCreateGraphicsPipelines(device.get_handle(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline.m_handle));

	return pipeline;
}


