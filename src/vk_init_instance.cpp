#include "app.hpp"

bool check_validation_layer()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layer_present(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layer_present.data());

    for (const char* layer_name : VALIDATION_LAYERS)
    {
        bool layer_found = false;
        for (const auto& layer_properties : layer_present)
        {
            if (!strcmp(layer_name, layer_properties.layerName))
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> get_required_exts()
{
    uint32_t glfwExtCount = 0;
    const char** glfwExts = nullptr;
    glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char*> exts(glfwExts, glfwExts + glfwExtCount);

    if (ENABLE_VALIDATION_LAYERS)
    {
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return exts;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                               void* pUserData)
{

    std::cerr << "[Vulkan Validation Layer] " << pCallbackData->pMessage << std::endl << std::endl;

    return VK_FALSE;
}

void App::create_instance()
{
    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer())
    {
        throw std::runtime_error("validation layer not avaliable\n");
    }

    vk::ApplicationInfo app_info{};
    app_info.pApplicationName = "vk app";
    app_info.pApplicationName = "vk app";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "vk engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    vk::DebugUtilsMessengerCreateInfoEXT debug_info;
    debug_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | //
                                 vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debug_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                             vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                             vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debug_info.pfnUserCallback = debug_cb;

    vk::InstanceCreateInfo create_info{};
    create_info.pApplicationInfo = &app_info;

    auto exts = get_required_exts();
    create_info.enabledExtensionCount = (uint32_t)exts.size();
    create_info.ppEnabledExtensionNames = exts.data();
    if (ENABLE_VALIDATION_LAYERS)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

  instance_ = vk::createInstance(create_info, nullptr);
    if (ENABLE_VALIDATION_LAYERS)
    {
        vk::DispatchLoaderDynamic instance_loader(instance_, vkGetInstanceProcAddr);
      debug_messenger_ = instance_.createDebugUtilsMessengerEXT(debug_info, nullptr, instance_loader);
    }

    if (glfwCreateWindowSurface(instance_, window, nullptr, (vk::SurfaceKHR::CType*)&surface_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}
