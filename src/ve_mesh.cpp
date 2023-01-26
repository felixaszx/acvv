#include "ve_mesh.hpp"

void VeMesh::load()
{
}

void VeMesh::draw()
{
}

std::array<VkVertexInputAttributeDescription, 5> VeMesh::get_attribute()
{
    std::array<VkVertexInputAttributeDescription, 5> attributes{};
    for (uint32_t i = 0; i < 5; i++)
    {
        attributes[i].binding = 0;
        attributes[i].location = i;
        attributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[i].offset = i * sizeof(glm::vec3) * positions.size();
    }

    return attributes;
}

VkVertexInputBindingDescription VeMesh::get_binding()
{
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = 0;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding;
}
