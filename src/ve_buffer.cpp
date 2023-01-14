#include "ve_buffer.hpp"

void ve_buffer_cpy(VeDeviceLayer& device_layer, VkBuffer src, VkBuffer dst, VkBufferCopy region)
{
    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer);
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);
    cmd.end(device_layer);
}