#include "app.hpp"

void App::run()
{
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
}

void App::init_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "vk", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
}

void App::init_vulkan()
{
    create_instance();
    setup_physical_device();
}

void App::cleanup()
{
    device_.destroy();
    vk::DispatchLoaderDynamic instance_loader(instance_, vkGetInstanceProcAddr);
    instance_.destroyDebugUtilsMessengerEXT(debug_messenger_, nullptr, instance_loader);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    instance_.destroy();
}

void App::main_loop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}