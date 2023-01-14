#include "acvv.hpp"

void Acvv::create_index_buffer()
{
    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VeBufferBase staging_buffer;
    VkBufferCreateInfo staging_buffer_info{};
    staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_buffer_info.size = size;
    staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer_, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_buffer,
                    nullptr);

    void* data = nullptr;
    vmaMapMemory(device_layer_, staging_buffer, &data);
    memcpy(data, indices.data(), size);
    vmaUnmapMemory(device_layer_, staging_buffer);

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(device_layer_, &buffer_info, &alloc_info, &index_buffer_, &index_buffer_, nullptr);

    copy_buffer(staging_buffer, index_buffer_, size);

    vmaDestroyBuffer(device_layer_, staging_buffer, staging_buffer);
}

void Acvv::create_vertex_buffer()
{
    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VeBufferBase staging_buffer;
    VkBufferCreateInfo staging_buffer_info{};
    staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_buffer_info.size = size;
    staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer_, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_buffer,
                    nullptr);

    void* data = nullptr;
    vmaMapMemory(device_layer_, staging_buffer, &data);
    memcpy(data, vertices.data(), size);
    vmaUnmapMemory(device_layer_, staging_buffer);

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(device_layer_, &buffer_info, &alloc_info, &vertex_buffer_, &vertex_buffer_, nullptr);

    copy_buffer(staging_buffer, vertex_buffer_, size);

    vmaDestroyBuffer(device_layer_, staging_buffer, staging_buffer);
}