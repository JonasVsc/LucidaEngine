#pragma once

// lib
#include <vulkan/vulkan.h>

// std
#include <string>

class Device;

class Shader {
public:

	Shader(Device& device, const std::string& filename);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;

	VkShaderModule get_module() const { return m_shader_module; }

private:

	Device& m_device;
	
	VkShaderModule m_shader_module = VK_NULL_HANDLE;
};