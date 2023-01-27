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
