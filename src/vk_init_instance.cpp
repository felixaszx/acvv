#include "app.hpp"

void App::init_vulkan()
{
    create_instance();
}

void App::cleanup()
{
}

std::vector<const char*> get_required_exts()
{
    uint32_t glfwExtCount = 0;
    const char** glfwExts = nullptr;
    glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char*> exts(glfwExts, glfwExts + glfwExtCount);

    if (enableValidationLayers)
    {
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return exts;
}

void App::create_instance()
{
    vk::ApplicationInfo app_info{};
    app_info.pApplicationName = "vk app";
    app_info.pApplicationName = "vk app";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "vk engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo create_info{};
    create_info.pApplicationInfo = &app_info;

    auto exts = get_required_exts();
    create_info.enabledExtensionCount = (uint32_t)exts.size();
    create_info.ppEnabledExtensionNames = exts.data();
    create_info.enabledLayerCount = 0;

    try
    {
        instance_ = vk::createInstance(create_info, nullptr);
    }
    catch (vk::SystemError err)
    {
        std::cerr << "[vk System Error] " << err.what() << std::endl;
    }
}
