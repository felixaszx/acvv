#ifndef VK_CMD_HPP
#define VK_CMD_HPP

#include "ve_multi_thread.hpp"
#include "ve_base.hpp"
#include "ve_device.hpp"

struct VeSingleTimeCmdBase : public MultiType<VkCommandPool, VkCommandBuffer>
{
    void begin(VeDeviceLayer& device_layer);
    void end(VeDeviceLayer& device_layer);

    void begin(VeDeviceLayer& device_layer, VkCommandPool pool);
    void end(VeDeviceLayer& device_layer, VkCommandPool pool);
};

struct VeCommandPoolBase : public MultiType<VkCommandPool>
{
    void create(VkDevice device, uint32_t family_index, VkCommandPoolCreateFlags flags);
    void destroy(VkDevice device);

    VkCommandBuffer allocate_buffer(VkDevice device, VkCommandBufferLevel level);
    std::vector<VkCommandBuffer> allocate_buffers(VkDevice device, uint32_t count, VkCommandBufferLevel level);
};

class VeMultiThreadCmdRecorder
{
  private:
    uint32_t curr_cmd = 0;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> cmds_{};
    VkCommandBufferInheritanceInfo inheritance_{};

    std::atomic_bool terminated = false;
    VeSemaphore begin_semaphore_{};
    VeSemaphore finish_semaphore_{};

  public:
    void create(VeDeviceLayer& device_layer, uint32_t cmd_count = 1);
    void destroy(VeDeviceLayer& device_layer);

    VkCommandBuffer get(uint32_t cmd_index = 0);
    void begin(VkCommandBufferInheritanceInfo inheritance, uint32_t cmd_index = 0);
    void wait();
    void wait_than_excute(VkCommandBuffer primary_cmd);
    void terminate();

    void record(const std::function<void(VkCommandBuffer)>& recording_func);
    void operator()(const std::function<void(VkCommandBuffer)>& recording_func);
};

#endif // VK_CMD_HPP
