#ifndef VE_MULTI_THREAD_HPP
#define VE_MULTI_THREAD_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <semaphore.h>
#include <assert.h>

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

#endif // VE_MULTI_THREAD_HPP
