#include "ve_cmd.hpp"

void VeSingleTimeCmdBase::begin(VeDeviceLayer& device_layer)
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = device_layer.queue_family_indices.graphics;
    vkCreateCommandPool(device_layer, &pool_info, nullptr, this->ptr());

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = *this;
    alloc_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(device_layer, &alloc_info, this->ptr());

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(*this, &begin_info);
}

void VeSingleTimeCmdBase::end(VeDeviceLayer& device_layer)
{
    vkEndCommandBuffer(*this);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = this->ptr();

    vkQueueSubmit(device_layer.graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device_layer.graphics_queue_);

    vkFreeCommandBuffers(device_layer, *this, 1, this->ptr());
    vkDestroyCommandPool(device_layer, *this, nullptr);
}
