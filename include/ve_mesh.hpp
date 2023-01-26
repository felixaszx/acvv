#ifndef VE_MESH_HPP
#define VE_MESH_HPP

#include <glm/glm.hpp>

#include "ve_buffer.hpp"

struct VeMeshVertex
{
    glm::vec3 positon_{};
    glm::vec3 normal_{};
    glm::vec3 uv_{};
    glm::vec3 color_{};
};

class VeMesh
{
  private:
    VeBufferBase VBO_{};
    VeBufferBase VBO_INSTANCE_{};

    VeBufferBase IBO_{};

  public:
    std::vector<VeMeshVertex> vertices{};
    std::vector<glm::vec3> indices{};

    void load();
    void draw();

    static std::array<VkVertexInputBindingDescription, 2> get_bindings();
    static std::array<VkVertexInputAttributeDescription, 5> get_attributes();
};

#endif // VE_MESH_HPP
