#include "ve_buffer.hpp"

void ve_buffer_cpy(VeDeviceLayer& device_layer, VkBuffer dst, VkBuffer src, VkBufferCopy region)
{
    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer);
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);
    cmd.end(device_layer);
}

void ve_buffer_cpy(VeDeviceLayer& device_layer, VkCommandPool pool, VkBuffer dst, VkBuffer src, VkBufferCopy region)
{
    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer, pool);
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);
    cmd.end(device_layer, pool);
}
