#ifndef VE_MULTI_THREAD_HPP
#define VE_MULTI_THREAD_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <semaphore.h>
#include <assert.h>

#include "ve_base.hpp"

class VeSemaphore
{
  private:
    sem_t semaphore_ = nullptr;

  public:
    void create(int initial_value);
    void destroy();

    void signal();
    void wait();
    bool try_wait();
    int get_value();
};

struct VeGpuSemaphore : public MultiType<VkSemaphore>
{
    using MultiType<VkSemaphore>::operator=;

    void create(VkDevice device);
    void destroy(VkDevice device);
};

struct VeGpuFence : public MultiType<VkFence>
{
    using MultiType<VkFence>::operator=;

    void create(VkDevice device, bool signal);
    void destroy(VkDevice device);
};

#endif // VE_MULTI_THREAD_HPP
