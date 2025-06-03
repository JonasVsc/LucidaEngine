#include "shader.h"

// core
#include "core/log.h"

#include "utils.h"

Shader::Shader(VkDevice device, const std::string& filename)
	: m_device{device}
{
	auto buffer = read_file(filename);

	std::vector<uint32_t> spirv = std::vector<uint32_t>(reinterpret_cast<uint32_t*>(buffer.data()),
		reinterpret_cast<uint32_t*>(buffer.data()) + buffer.size() / sizeof(uint32_t));
	
	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = spirv.size() * sizeof(uint32_t),
		.pCode = spirv.data(),
	};

	VK_CHECK(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &m_shader_module));
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_device, m_shader_module, nullptr);
}
