#include "acvv.hpp"

void Acvv::create_instance()
{
    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support())
    {
        throw std::runtime_error("validation layer is not supported\n");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "vk app";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "vk engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_creat_info{};
    debug_utils_creat_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_creat_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | //
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_creat_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |    //
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | //
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_creat_info.pfnUserCallback = debug_cb;

    auto exts = get_required_exts();
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.enabledExtensionCount = castt(uint32_t, exts.size());
    instance_create_info.ppEnabledExtensionNames = exts.data();

    if (ENABLE_VALIDATION_LAYERS)
    {
        instance_create_info.enabledLayerCount = castt(uint32_t, VALIDATION_LAYERS.size());
        instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        instance_create_info.pNext = castt(VkDebugUtilsMessengerCreateInfoEXT*, &debug_utils_creat_info);
    }
    else
    {
        instance_create_info.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&instance_create_info, nullptr, &instance_) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create vulkan instance\n");
    }

    if (ENABLE_VALIDATION_LAYERS)
    {
        auto load_func = load_ext_function<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr, instance_,
                                                                               "vkCreateDebugUtilsMessengerEXT");
        if (load_func(instance_, &debug_utils_creat_info, nullptr, &messenger_) != VK_SUCCESS)
        {

            throw std::runtime_error("Do not create validation layers\n");
        }
    }

    if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create window surface\n");
    }
}