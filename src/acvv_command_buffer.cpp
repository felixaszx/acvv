#include "acvv.hpp"

void Acvv::create_sync_objs()
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    get_image_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    image_render_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    frame_fence_.resize(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkCreateSemaphore(device_layer_, &semaphore_info, nullptr, &get_image_semaphores_[i]);
        vkCreateSemaphore(device_layer_, &semaphore_info, nullptr, &image_render_semaphores_[i]);
        vkCreateFence(device_layer_, &fence_info, nullptr, &frame_fence_[i]);
    }
}

void Acvv::create_command_buffer()
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = device_layer_.queue_family_indices.graphics;
    vkCreateCommandPool(device_layer_, &pool_info, nullptr, &command_pool_);

    command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = castt(uint32_t, command_buffers_.size());
    vkAllocateCommandBuffers(device_layer_, &alloc_info, command_buffers_.data());
}

void Acvv::create_depth_image()
{
    VkImageCreateInfo depth_create_info{};
    depth_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_create_info.imageType = VK_IMAGE_TYPE_2D;
    depth_create_info.extent.width = casts(uint32_t, swapchain_.extend_.width);
    depth_create_info.extent.height = casts(uint32_t, swapchain_.extend_.height);
    depth_create_info.extent.depth = 1;
    depth_create_info.mipLevels = 1;
    depth_create_info.arrayLayers = 1;
    depth_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    depth_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depth_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VmaAllocationCreateInfo depth_alloc_info{};
    depth_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateImage(device_layer_, &depth_create_info, &depth_alloc_info, &depth_image_, &depth_image_, nullptr);
    VkImageViewCreateInfo depth_view_info{};
    depth_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_view_info.image = depth_image_;
    depth_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_view_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    depth_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_view_info.subresourceRange.baseMipLevel = 0;
    depth_view_info.subresourceRange.levelCount = 1;
    depth_view_info.subresourceRange.baseArrayLayer = 0;
    depth_view_info.subresourceRange.layerCount = 1;
    vkCreateImageView(device_layer_, &depth_view_info, nullptr, &depth_image_);

    transition_image_layout(depth_image_, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}