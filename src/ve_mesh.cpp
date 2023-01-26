#include "ve_mesh.hpp"

void VeMesh::load()
{
}

void VeMesh::draw()
{
}

std::array<VkVertexInputAttributeDescription, 5> VeMesh::get_attributes()
{
    std::array<VkVertexInputAttributeDescription, 5> attributes{};
    for (uint32_t i = 0; i < 4; i++)
    {
        attributes[i].binding = 0;
        attributes[i].location = i;
        attributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    }

    attributes[0].offset = offsetof(VeMeshVertex, positon_);
    attributes[1].offset = offsetof(VeMeshVertex, normal_);
    attributes[2].offset = offsetof(VeMeshVertex, uv_);
    attributes[3].offset = offsetof(VeMeshVertex, color_);

    for (uint32_t i = 4; i < 8; i++)
    {
        attributes[i].binding = 1;
        attributes[i].location = i;
        attributes[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributes[i].offset = (i - 4) * sizeof(glm::vec4);
    }

    return attributes;
}

std::array<VkVertexInputBindingDescription, 2> VeMesh::get_bindings()
{
    std::array<VkVertexInputBindingDescription, 2> binding{};

    binding[0].binding = 0;
    binding[0].stride = sizeof(VeMeshVertex);
    binding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    binding[1].binding = 1;
    binding[1].stride = sizeof(glm::mat4);
    binding[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return binding;
}
