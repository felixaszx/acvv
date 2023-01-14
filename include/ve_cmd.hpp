#ifndef VK_CMD_HPP
#define VK_CMD_HPP

#include "ve_base.hpp"
#include "ve_device.hpp"

class VeSingleTimeCmdBase : public MultiType<VkCommandPool, VkCommandBuffer>
{
  private:
    using MultiType<VkCommandPool, VkCommandBuffer>::operator=;

  public:
    void begin(VeDeviceLayer& device_layer);
    void end(VeDeviceLayer& device_layer);

    void begin(VeDeviceLayer& device_layer, VkCommandPool pool);
    void end(VeDeviceLayer& device_layer, VkCommandPool pool);
};

#endif // VK_CMD_HPP
