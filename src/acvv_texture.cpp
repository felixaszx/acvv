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

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = castt(uint32_t, width);
    image_create_info.extent.height = castt(uint32_t, height);
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkCreateImage(device_, &image_create_info, nullptr, &texture_image_) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create image\n");
    }
}
