#include "ve_camera.hpp"

void VeCameraBase::create(VeDeviceLayer device_layer)
{
    VkBufferCreateInfo ubo_info{};
    ubo_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ubo_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ubo_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ubo_info.size = sizeof(matrices);
    VmaAllocationCreateInfo ubo_alloc_info{};
    ubo_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    ubo_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer, &ubo_info, &ubo_alloc_info, &uniform_buffer, &uniform_buffer, nullptr);
    vmaMapMemory(device_layer, uniform_buffer, &uniform_map);

    descriptor_buffer_info.buffer = uniform_buffer;
    descriptor_buffer_info.offset = 0;
    descriptor_buffer_info.range = VK_WHOLE_SIZE;
}

void VeCameraBase::destroy(VeDeviceLayer device_layer)
{
    uniform_map = nullptr;
    vmaUnmapMemory(device_layer, uniform_buffer);
    vmaDestroyBuffer(device_layer, uniform_buffer, uniform_buffer);
}

void VeCameraBase::update(VkExtent2D viewport_extent)
{
    front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front);
    up = glm::normalize(up);

    matrices.view = glm::lookAt(position, position + front, up);
    matrices.proj = glm::perspective(glm::radians(fov),                                     //
                                     (float)viewport_extent.width / viewport_extent.height, //
                                     near, far);
    matrices.proj[1][1] *= -1;
    memcpy(uniform_map, &matrices, sizeof(matrices));
}

glm::vec3 VeCameraBase::right() const
{
    return glm::normalize(glm::cross(front, up));
}
