#include "ve_multi_thread.hpp"

void VeSemaphore::create(int initial_value)
{
    assert(semaphore_ == nullptr);
    sem_init(&semaphore_, 0, initial_value);
}

void VeSemaphore::destroy()
{
    assert(semaphore_ != nullptr);
    sem_destroy(&semaphore_);
    semaphore_ = nullptr;
}

void VeSemaphore::signal()
{
    assert(semaphore_ != nullptr);
    sem_post(&semaphore_);
}

void VeSemaphore::wait()
{
    assert(semaphore_ != nullptr);
    sem_wait(&semaphore_);
}

bool VeSemaphore::try_wait()
{
    assert(semaphore_ != nullptr);
    return !sem_trywait(&semaphore_);
}

int VeSemaphore::get_value()
{
    int value;
    assert(semaphore_ != nullptr);
    sem_getvalue(&semaphore_, &value);
    return value;
}

void VeGpuSemaphore::create(VkDevice device)
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device, &semaphore_info, nullptr, this->ptr());
}

void VeGpuSemaphore::destroy(VkDevice device)
{
    vkDestroySemaphore(device, *this, nullptr);
}

void VeGpuFence::create(VkDevice device, bool signal)
{
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = signal ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    vkCreateFence(device, &fence_info, nullptr, this->ptr());
}

void VeGpuFence::destroy(VkDevice device)
{
    vkDestroyFence(device, *this, nullptr);
}
