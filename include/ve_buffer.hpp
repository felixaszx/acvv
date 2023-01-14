#ifndef VE_BUFFER_HPP
#define VE_BUFFER_HPP

#include "ve_base.hpp"
#include "ve_cmd.hpp"

using VeBufferBase = MultiType<VkBuffer, VkDeviceMemory, VmaAllocation>;

void ve_buffer_cpy(VeDeviceLayer& device_layer, VkBuffer dst, VkBufferCopy region, VkBuffer src);

#endif // VE_BUFFER_HPP
