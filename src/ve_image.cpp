#include "ve_image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

VeImagePixels load_pixel(const std::string file_name, uint32_t channel)
{
    VeImagePixels image_pixels{};
    int w, h, chan;

    image_pixels.pixels = stbi_load("res/texture.jpg", &w, &h, &chan, channel);
    if (image_pixels.pixels == nullptr)
    {
        return VeImagePixels();
    }
    image_pixels.width = casts(uint32_t, w);
    image_pixels.height = casts(uint32_t, h);
    image_pixels.channel = casts(uint32_t, chan);
    image_pixels.size = image_pixels.width * image_pixels.height * channel;

    return image_pixels;
}

void free_pixel(VeImagePixels& pixel_data)
{
    stbi_image_free(pixel_data.pixels);
    pixel_data.pixels = nullptr;
}

void VeTextureBase::copy_buffer_to_image(VkBuffer buffer, VeDeviceLayer device_layer, VkCommandPool pool)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {pixels_data.width, pixels_data.height, 1};

    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer, pool);
    vkCmdCopyBufferToImage(cmd, buffer, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    cmd.end(device_layer, pool);
}

void VeTextureBase::transit_to_dst(VeDeviceLayer device_layer, VkCommandPool pool)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = *this;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer, pool);
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, //
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    cmd.end(device_layer, pool);
}

void VeTextureBase::transit_to_shader(VeDeviceLayer device_layer, VkCommandPool pool)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = *this;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer, pool);
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, //
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    cmd.end(device_layer, pool);
}

VkSampler VeTextureBase::default_sampler = VK_NULL_HANDLE;

void VeTextureBase::set_default_sampler(VkDevice device)
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 4.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    vkCreateSampler(device, &sampler_info, nullptr, &default_sampler);
}

VeTextureBase::VeTextureBase(const std::string& file_name)
{
    pixels_data = load_pixel(file_name, STBI_rgb_alpha);
}

void VeTextureBase::create(VeDeviceLayer device_layer, VkCommandPool pool)
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = pixels_data.width;
    image_info.extent.height = pixels_data.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateImage(device_layer, &image_info, &alloc_info, this->ptr(), this->ptr(), nullptr);

    void* staging_buffer_map = nullptr;
    VeBufferBase staging_buffer{};
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.size = pixels_data.size;
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer, &buffer_info, &staging_alloc_info, &staging_buffer, &staging_buffer, nullptr);

    vmaMapMemory(device_layer, staging_buffer, &staging_buffer_map);
    memcpy(staging_buffer_map, pixels_data.pixels, pixels_data.size);
    vmaUnmapMemory(device_layer, staging_buffer);
    free_pixel(pixels_data);

    transit_to_dst(device_layer, pool);
    copy_buffer_to_image(staging_buffer, device_layer, pool);
    transit_to_shader(device_layer, pool);

    vmaDestroyBuffer(device_layer, staging_buffer, staging_buffer);
}

void VeTextureBase::destroy(VeDeviceLayer device_layer)
{
    vkDestroyImageView(device_layer, *this, nullptr);
    vmaDestroyImage(device_layer, *this, *this);
}

void VeTextureBase::create_image_view(VkDevice device)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = *this;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    vkCreateImageView(device, &view_info, nullptr, this->ptr());
}
