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
    window = glfwCreateWindow(WIDTH, HEIGHT, "vk", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow* window, int width, int height)
                                   {
                                       App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
                                       while (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                                       {
                                           glfwWaitEvents();
                                       }
                                       app->reset_swapchain();
                                   });
}

void App::init_vulkan()
{
    create_instance();
    setup_physical_device();

    setup_swapchain();
    setup_swapchain_imageview();

    setup_render_pass();
    setup_descriptor_set_layout();
    setup_graphic_pipeline();

    setup_sync_objs();
    setup_framebuffers();

    setup_command_buffer();
    setup_vertex_buffer();
    setup_index_buffer();
    setup_uniform_buffer();
    setup_descriptor_pool();
}

void App::cleanup()
{
    clear_swapchain();

    device_.destroyDescriptorPool(descriptor_pool);
    device_.destroyDescriptorSetLayout(descriptor_set_layout);

    device_.destroyBuffer(index_buffer);
    device_.freeMemory(index_buffer_memory);

    device_.destroyBuffer(vertex_buffer);
    device_.freeMemory(vertex_buffer_memory);

    device_.destroyPipeline(graphics_pipeline);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        device_.destroyBuffer(uniform_buffers[i]);
        device_.freeMemory(uniform_buffers_memory[i]);
    }
    device_.destroyPipelineLayout(pipeline_layout);
    device_.destroyRenderPass(render_pass);

    for (auto& render_semaphore : render_semaphores)
    {
        device_.destroySemaphore(render_semaphore);
    }
    for (auto& image_semaphore : image_semaphores)
    {
        device_.destroySemaphore(image_semaphore);
    }
    for (auto& in_flight : in_flights)
    {
        device_.destroyFence(in_flight);
    }
    device_.destroyCommandPool(command_pool);

    device_.destroy();

    if (ENABLE_VALIDATION_LAYERS)
    {
        vk::DispatchLoaderDynamic instance_loader(instance_, vkGetInstanceProcAddr);
        instance_.destroyDebugUtilsMessengerEXT(debug_messenger_, nullptr, instance_loader);
    }
    instance_.destroySurfaceKHR(surface_);
    instance_.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::main_loop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw_frame();
    }

    device_.waitIdle();
}
