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