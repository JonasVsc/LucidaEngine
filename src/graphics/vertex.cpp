#include "vertex.h"

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_descriptions()
{
	std::vector<VkVertexInputBindingDescription> bindings(1);

	bindings[0].binding = 0;
	bindings[0].stride = sizeof(Vertex);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindings;
}

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributes(2);

	attributes[0].binding = 0;
	attributes[0].location = 0;
	attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributes[0].offset = offsetof(Vertex, position);

	attributes[1].binding = 0;
	attributes[1].location = 1;
	attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributes[1].offset = offsetof(Vertex, color);

	return attributes;
}
