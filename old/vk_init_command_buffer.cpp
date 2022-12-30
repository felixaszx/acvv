#include "app.hpp"

void App::setup_sync_objs()
{
    vk::SemaphoreCreateInfo semaphore_info{};
    vk::FenceCreateInfo fence_info{};
    fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

    image_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flights.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        image_semaphores[i] = device_.createSemaphore(semaphore_info);
        render_semaphores[i] = device_.createSemaphore(semaphore_info);
        in_flights[i] = device_.createFence(fence_info);
    }
}

void App::setup_command_buffer()
{
    vk::CommandPoolCreateInfo pool_info{};
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_info.queueFamilyIndex = queue_family_indices.graphic;
    command_pool = device_.createCommandPool(pool_info);

    command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.commandPool = command_pool;
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandBufferCount = (uint32_t)command_buffers.size();
   command_buffers = device_.allocateCommandBuffers(alloc_info);
}