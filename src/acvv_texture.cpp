#include "acvv.hpp"

void Acvv::create_texture_image()
{
    int width = 0;
    int height = 0;
    int channel = 0;
    stbi_uc* pixel = stbi_load("res/texture.jpg", &width, &height, &channel, STBI_rgb_alpha);
    if (pixel == nullptr)
    {
        throw std::runtime_error("Do not load texture\n");
    }
    VkDeviceSize size = width * height * channel;

    VkBuffer stagine_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;

    create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,                                     //
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //
                  stagine_buffer, staging_memory);

    void* data;
    vkMapMemory(device_, staging_memory, 0, size, 0, &data);
    memcpy(data, pixel, castt(size_t, size));
    vkUnmapMemory(device_, staging_memory);

    stbi_image_free(pixel);
}
