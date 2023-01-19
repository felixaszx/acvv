#ifndef VE_DEVICE_HPP
#define VE_DEVICE_HPP

#include <set>
#include <algorithm>

#include "ve_base.hpp"

using VeDeviceBase = MultiType<VkPhysicalDevice, VkDevice, VmaAllocator>;

struct QueueFamilyIndex
{
    uint32_t graphics;
    uint32_t present;
};

struct VeDeviceLayer : public VeDeviceBase
{
    const std::vector<const char*> REQUIRED_DEVICE_EXTS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    QueueFamilyIndex queue_family_indices{};
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    using VeDeviceBase::operator=;

    void find_device(VeBaseLayer& base_Layer);
    void create(VeBaseLayer& base_Layer);
    void destroy();
};

#endif // VE_DEVICE_HPP
