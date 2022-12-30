#include "app.hpp"

#include <optional>
#include <set>

void App::setup_physical_device()
{
    auto physical_devices = instance_.enumeratePhysicalDevices();

    std::optional<uint32_t> graphic_family;
    std::optional<uint32_t> present_family;

    bool device_ready = false;
    for (const auto& device : physical_devices)
    {
        graphic_family.reset();
        present_family.reset();

        auto queue_properties = device.getQueueFamilyProperties();
        auto supported_exts = device.enumerateDeviceExtensionProperties();

        bool exts_supported = false;
        bool swapchain_adaquate = false;

        std::set<std::string> requiredExtensions(DEVICE_EXTS.begin(), DEVICE_EXTS.end());
        for (const auto& extension : supported_exts)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        if (requiredExtensions.empty())
        {
            exts_supported = true;
        }

        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
        formats = device.getSurfaceFormatsKHR(surface_);
        present_modes = device.getSurfacePresentModesKHR(surface_);
        swapchain_adaquate = !(formats.empty() || present_modes.empty());

        for (uint32_t index = 0; index < queue_properties.size(); index++)
        {
            vk::Bool32 present_support = device.getSurfaceSupportKHR(index, surface_);

            if (queue_properties[index].queueFlags & vk::QueueFlagBits::eGraphics && present_support)
            {
                graphic_family = index;
                present_family = index;
                break;
            }
        }

        if (graphic_family.has_value() && present_family.has_value() && exts_supported && swapchain_adaquate)
        {
            device_ready = true;
            physical_device_ = device;
            std::cerr << "Using physical device name: " << device.getProperties().deviceName << std::endl << std::endl;
            break;
        }
    }

    if (!device_ready)
    {
        throw std::runtime_error("No Device is suitable for Vulkan\n");
    }

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {graphic_family.value(), present_family.value()};
    queue_family_indices = {graphic_family.value(), present_family.value()};

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

    device_ = physical_device_.createDevice(create_info);

    graphics_queue_ = device_.getQueue(graphic_family.value(), 0);
    present_queue_ = device_.getQueue(present_family.value(), 0);
}