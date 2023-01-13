#include "acvv.hpp"

void Acvv::create_index_buffer()
{
    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VkBuffer staging_buffer{};
    VkDeviceMemory stagine_memory{};
    create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //
                  staging_buffer, stagine_memory);

    void* data = nullptr;
    vkMapMemory(device_layer_, stagine_memory, 0, size, 0, &data);
    memcpy(data, indices.data(), size);
    vkUnmapMemory(device_layer_, stagine_memory);

    create_buffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //
                  index_buffer_, index_buffer_memory_);

    copy_buffer(staging_buffer, index_buffer_, size);

    vkDestroyBuffer(device_layer_, staging_buffer, nullptr);
    vkFreeMemory(device_layer_, stagine_memory, nullptr);
}

void Acvv::create_vertex_buffer()
{
    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer staging_buffer{};
    VkDeviceMemory stagine_memory{};
    create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //
                  staging_buffer, stagine_memory);

    void* data = nullptr;
    vkMapMemory(device_layer_, stagine_memory, 0, size, 0, &data);
    memcpy(data, vertices.data(), size);
    vkUnmapMemory(device_layer_, stagine_memory);

    create_buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //
                  vertex_buffer_, vertex_buffer_memory_);

    copy_buffer(staging_buffer, vertex_buffer_, size);

    vkDestroyBuffer(device_layer_, staging_buffer, nullptr);
    vkFreeMemory(device_layer_, stagine_memory, nullptr);
}