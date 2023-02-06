#ifndef VE_MESH_HPP
#define VE_MESH_HPP

#include "ve_cmd.hpp"
#include "ve_buffer.hpp"
#include "ass.hpp"
#include "glms.hpp"

struct VeMeshVertex
{
    glm::vec3 positon_{};
    glm::vec3 normal_{};
    glm::vec3 uv_{};
    glm::vec3 color_{1.0f, 1.0f, 1.0f};
};

class VeMesh
{
  private:
    VeBufferBase vert_buffer_{};
    VeBufferBase vert_buffer_INSTANCE_{};
    std::vector<VeMeshVertex> vertices_{};

    VeBufferBase index_buffer_{};
    std::vector<uint32_t> indices_{};
    std::vector<uint32_t> indices_count_{};

    std::vector<size_t> vert_buffer_offsets_{};
    std::vector<size_t> indices_buffer_offsets_{};

    void* instance_mapping_ = nullptr;
    VeBufferBase instance_buffer_{};
    uint32_t update_size_ = 1;

  public:
    const uint32_t MAX_INSTANCE;
    uint32_t instance_count_ = 1;
    std::vector<glm::mat4> instances_{};

    VeMesh(const std::string file_path, uint32_t max_instance);

    void create(VeDeviceLayer device_layer);
    void draw(VkCommandBuffer cmd);
    void update();
    void destroy(VeDeviceLayer device_layer);

    static std::array<VkVertexInputBindingDescription, 2> get_bindings();
    static std::array<VkVertexInputAttributeDescription, 8> get_attributes();
};

#endif // VE_MESH_HPP
