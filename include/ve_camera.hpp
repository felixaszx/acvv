#ifndef VE_CAMERA_HPP
#define VE_CAMERA_HPP

#include "ve_base.hpp"
#include "ve_buffer.hpp"
#include "glms.hpp"

class VeCameraBase
{
  private:
    VeBufferBase uniform_buffer{};
    void* uniform_map = nullptr;

  public:
    VkDescriptorBufferInfo descriptor_buffer_info{};

    struct CameraUniformBuffer
    {
        glm::mat4 view{};
        glm::mat4 proj{};
    };
    CameraUniformBuffer matrices{};

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    glm::vec3 front{};

    float yaw = 0.0f;
    float pitch = 0.0f;
    float fov = 45.0f;

    float near = 0.5f;
    float far = 1000.0f;

    void create(VeDeviceLayer device_layer);
    void destroy(VeDeviceLayer device_layer);

    void update(VkExtent2D viewport_extent);
    glm::vec3 right() const;
};

#endif // VE_CAMERA_HPP
