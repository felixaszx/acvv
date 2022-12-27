#include "app.hpp"

#include <optional>
#include <set>

void App::setup_physical_device()
{
    auto physical_devices = instance_.enumeratePhysicalDevices();

    std::optional<uint32_t> graphic_family;
    std::optional<uint32_t> present_family;

    for (auto& device : physical_devices)
    {
        graphic_family.reset();
        present_family.reset();

        auto queue_properties = device.getQueueFamilyProperties();
        auto supported_exts = device.enumerateDeviceExtensionProperties();

        for (uint32_t index = 0; index < queue_properties.size(); index++)
        {
            if (queue_properties[index].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphic_family = index;
                break;
            }
        }

        if (graphic_family.has_value())
        {
            physical_device_ = device;
            break;
        }
    }

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {graphic_family.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        vk::DeviceQueueCreateInfo queue_create_info{};
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;

        queue_create_infos.push_back(queue_create_info);
    }

    vk::PhysicalDeviceFeatures device_features{};

    vk::DeviceCreateInfo create_info{};
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = (uint32_t)queue_create_infos.size();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = (uint32_t)DEVICE_EXTS.size();
    create_info.ppEnabledExtensionNames = DEVICE_EXTS.data();

    if (ENABLE_VALIDATION_LAYERS)
    {
        create_info.enabledExtensionCount = (uint32_t)VALIDATION_LAYERS.size();
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    VK_CHECK(device_ = physical_device_.createDevice(create_info))
}