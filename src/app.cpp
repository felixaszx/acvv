#include "app.hpp"

void App::run()
{
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
}

void App::main_loop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void App::init_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "vk", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
}
