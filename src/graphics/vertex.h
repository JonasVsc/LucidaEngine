#pragma once

// lib
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// std
#include <vector>

struct Vertex {

	glm::vec3 position;
	glm::vec3 color;

	static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
	static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
};