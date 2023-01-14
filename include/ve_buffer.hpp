#ifndef VE_BUFFER_HPP
#define VE_BUFFER_HPP

#include "ve_base.hpp"
#include "ve_cmd.hpp"

using VeBufferBase = MultiType<VkBuffer, VkDeviceMemory, VmaAllocation>;

void ve_buffer_cpy(VeDeviceLayer& device_layer, VkBuffer dst, VkBuffer src, VkBufferCopy region);
void ve_buffer_cpy(VeDeviceLayer& device_layer, VkCommandPool pool, VkBuffer dst, VkBuffer src, VkBufferCopy region);

#endif // VE_BUFFER_HPP
