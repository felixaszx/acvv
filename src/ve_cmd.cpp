#include "ve_cmd.hpp"

void VeSingleTimeCmdBase::begin(VeDeviceLayer& device_layer)
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = device_layer.queue_family_indices.graphics;
    vkCreateCommandPool(device_layer, &pool_info, nullptr, this->ptr());

    begin(device_layer, *this);
}

void VeSingleTimeCmdBase::end(VeDeviceLayer& device_layer)
{
    end(device_layer, *this);
    vkDestroyCommandPool(device_layer, *this, nullptr);
}

void VeSingleTimeCmdBase::begin(VeDeviceLayer& device_layer, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = pool;
    alloc_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(device_layer, &alloc_info, this->ptr());

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(*this, &begin_info);
}

void VeSingleTimeCmdBase::end(VeDeviceLayer& device_layer, VkCommandPool pool)
{
    vkEndCommandBuffer(*this);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = this->ptr();

    vkQueueSubmit(device_layer.graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device_layer.graphics_queue_);

    vkFreeCommandBuffers(device_layer, pool, 1, this->ptr());
}

void VeMultiThreadRecord::create(VeDeviceLayer& device_layer, uint32_t cmd_count)
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = device_layer.queue_family_indices.graphics;
    vkCreateCommandPool(device_layer, &pool_info, nullptr, &pool_);

    cmds_.resize(cmd_count);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandPool = pool_;
    alloc_info.commandBufferCount = cmd_count;
    vkAllocateCommandBuffers(device_layer, &alloc_info, cmds_.data());

    begin_semaphores_.resize(cmd_count);
    finish_semaphores_.resize(cmd_count);
    for (uint32_t i = 0; i < cmd_count; i++)
    {
        sem_init(&begin_semaphores_[i], 0, 0);
        sem_init(&finish_semaphores_[i], 0, 0);
    }
}

void VeMultiThreadRecord::destroy(VeDeviceLayer& device_layer)
{
    for (uint32_t i = 0; i < begin_semaphores_.size(); i++)
    {
        sem_destroy(&begin_semaphores_[i]);
        sem_destroy(&finish_semaphores_[i]);
    }
    vkDestroyCommandPool(device_layer, pool_, nullptr);
}

VkCommandBuffer VeMultiThreadRecord::get(uint32_t cmd_index)
{
    return cmds_[cmd_index];
}

void VeMultiThreadRecord::begin(VkCommandBufferInheritanceInfo inheritance, uint32_t cmd_index)
{
    inheritance_ = inheritance;
    curr_cmd = cmd_index;
    sem_post(&begin_semaphores_[cmd_index]);
}

void VeMultiThreadRecord::wait()
{
    sem_wait(&finish_semaphores_[curr_cmd]);
}

void VeMultiThreadRecord::excute(VkCommandBuffer primary_cmd)
{
    wait();
    vkCmdExecuteCommands(primary_cmd, 1, &cmds_[curr_cmd]);
}

void VeMultiThreadRecord::terminate()
{
    terminated = true;
    begin({});
}

void VeMultiThreadRecord::operator()(const std::function<void(VkCommandBuffer)>& recording_func)
{
    while (true)
    {
        sem_wait(&begin_semaphores_[curr_cmd]);
        if (terminated)
        {
            return;
        }

        vkResetCommandBuffer(cmds_[curr_cmd], 0);
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        begin_info.pInheritanceInfo = &inheritance_;

        if (vkBeginCommandBuffer(cmds_[curr_cmd], &begin_info) != VK_SUCCESS)
        {
            std::cout << "This secondary commandbuffer do not start" << std::endl;
        }

        recording_func(cmds_[curr_cmd]);

        if (vkEndCommandBuffer(cmds_[curr_cmd]) != VK_SUCCESS)
        {
            std::cout << "This secondary commandbuffer do not end" << std::endl;
        }

        sem_post(&finish_semaphores_[curr_cmd]);
    }
}
