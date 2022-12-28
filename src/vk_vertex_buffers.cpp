#include "app.hpp"

void App::setup_index_buffer()
{
    vk::DeviceSize size = sizeof(indices[0]) * indices.size();

    vk::Buffer staging_buffer{};
    vk::DeviceMemory stagine_memory{};
    create_buffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, //
                  staging_buffer, stagine_memory);

    void* data = nullptr;
    data = device_.mapMemory(stagine_memory, 0, size);
    memcpy(data, indices.data(), size);
    device_.unmapMemory(stagine_memory);

    create_buffer(size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                  vk::MemoryPropertyFlagBits::eDeviceLocal, //
                  index_buffer, index_buffer_memory);

    copy_buffer(staging_buffer, index_buffer, size);

    device_.destroyBuffer(staging_buffer);
    device_.freeMemory(stagine_memory);
}

void App::setup_vertex_buffer()
{
    vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();

    vk::Buffer staging_buffer{};
    vk::DeviceMemory stagine_memory{};
    create_buffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, //
                  staging_buffer, stagine_memory);

    void* data = nullptr;
    data = device_.mapMemory(stagine_memory, 0, size);
    memcpy(data, vertices.data(), size);
    device_.unmapMemory(stagine_memory);

    create_buffer(size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                  vk::MemoryPropertyFlagBits::eDeviceLocal, //
                  vertex_buffer, vertex_buffer_memory);

    copy_buffer(staging_buffer, vertex_buffer, size);

    device_.destroyBuffer(staging_buffer);
    device_.freeMemory(stagine_memory);
}