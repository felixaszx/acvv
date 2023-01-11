#include "acvv.hpp"

void Acvv::setup_device()
{
    uint32_t physical_deviec_count = 0;
    vkEnumeratePhysicalDevices(instance_, &physical_deviec_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(physical_deviec_count);
    vkEnumeratePhysicalDevices(instance_, &physical_deviec_count, physical_devices.data());

    bool devie_found = false;
    for (const VkPhysicalDevice& device : physical_devices)
    {
        uint32_t queue_properties_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_properties_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_properties(queue_properties_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_properties_count, queue_properties.data());

        uint32_t supported_exts_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_exts_count, nullptr);
        std::vector<VkExtensionProperties> supported_exts(supported_exts_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &supported_exts_count, supported_exts.data());

        bool exts_supported = false;
        bool swapchain_adaquate = false;

        VkPhysicalDeviceFeatures supported_feature{};
        vkGetPhysicalDeviceFeatures(device, &supported_feature);

        std::set<std::string> required_exts(REQUIRED_DEVICE_EXTS.begin(), REQUIRED_DEVICE_EXTS.end());
        for (const VkExtensionProperties& ext : supported_exts)
        {
            required_exts.erase(ext.extensionName);
        }
        exts_supported = required_exts.empty();

        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &surface_format_count, nullptr);

        uint32_t present_modes_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_modes_count, nullptr);
        swapchain_adaquate = (present_modes_count != 0 && surface_format_count != 0);

        for (uint32_t index = 0; index < queue_properties.size(); index++)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface_, &present_support);

            if (queue_properties[index].queueFlags & VK_QUEUE_GRAPHICS_BIT //
                && present_support && swapchain_adaquate && exts_supported //
                && supported_feature.samplerAnisotropy)
            {
                queue_family_indices.graphics = index;
                queue_family_indices.present = index;
                devie_found = true;
                physical_device_ = device;

                break;
            }
        }
    }

    if (!devie_found)
    {
        throw std::runtime_error("No Device is suitable for Vulkan\n");
    }

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    std::set<uint32_t> unique_queue_families = {queue_family_indices.graphics, queue_family_indices.present};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;

        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.queueCreateInfoCount = castt(uint32_t, queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = castt(uint32_t, REQUIRED_DEVICE_EXTS.size());
    device_create_info.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTS.data();

    if (ENABLE_VALIDATION_LAYERS)
    {
        device_create_info.enabledLayerCount = castt(uint32_t, VALIDATION_LAYERS.size());
        device_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    if (vkCreateDevice(physical_device_, &device_create_info, nullptr, &device_) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create vulkan device\n");
    }

    vkGetDeviceQueue(device_, queue_family_indices.graphics, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, queue_family_indices.present, 0, &present_queue_);

    VmaVulkanFunctions vma_functions{};
    vma_functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vma_functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
    vma_functions.vkGetDeviceBufferMemoryRequirements = &vkGetDeviceBufferMemoryRequirements;
    vma_functions.vkGetDeviceImageMemoryRequirements = &vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo vma_create_info{};
    vma_create_info.vulkanApiVersion = VK_API_VERSION_1_3;
    vma_create_info.pVulkanFunctions = &vma_functions;
    vma_create_info.instance = instance_;
    vma_create_info.physicalDevice = physical_device_;
    vma_create_info.device = device_;

    vmaCreateAllocator(&vma_create_info, &vma_allocator_);
}
