#ifndef VE_MESH_HPP
#define VE_MESH_HPP

#include <glm/glm.hpp>

#include "ve_buffer.hpp"

class VeMesh
{
  private:
    VeBufferBase VBO_POSITION{};
    VeBufferBase VBO_NORMAL{};
    VeBufferBase VBO_UVS{};
    VeBufferBase VBO_COLORS{};

    VeBufferBase VBO_INSTANCE{};
    VeBufferBase IBO_INSTANCE{};

  public:
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec3> uvs{};
    std::vector<glm::vec3> colors{};

    std::vector<glm::vec3> indices{};

    void load();
    void draw();
    std::array<VkVertexInputAttributeDescription, 5> get_attribute();

    static VkVertexInputBindingDescription get_binding();
};

#endif // VE_MESH_HPP
