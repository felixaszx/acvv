#include "acvv.hpp"

void Acvv::run()
{
    init_window();
    init_vulkan();
    main_loop();
    cleanup();
}

void Acvv::init_window()
{
    base_layer_.create("vk");
    glfwSetWindowUserPointer(base_layer_, this);
    glfwSetFramebufferSizeCallback(base_layer_,
                                   [](GLFWwindow* window, int width, int height)
                                   {
                                       Acvv* app = reinterpret_cast<Acvv*>(glfwGetWindowUserPointer(window));
                                       while (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                                       {
                                           glfwWaitEvents();
                                       }

                                       vkDeviceWaitIdle(app->device_layer_);
                                       for (auto framebuffer : app->swapchain_framebuffers_)
                                       {
                                           vkDestroyFramebuffer(app->device_layer_, framebuffer, nullptr);
                                       }
                                       app->swapchain_.destroy(app->device_layer_);

                                       app->swapchain_.create(app->base_layer_, app->base_layer_, app->device_layer_);
                                       app->swapchain_.create_image_view(app->device_layer_);
                                       app->create_depth_image();
                                       app->create_framebuffers();
                                   });
}

void Acvv::init_vulkan()
{
    device_layer_.create(base_layer_);
    swapchain_.create(base_layer_, base_layer_, device_layer_);
    swapchain_.create_image_view(device_layer_);

    create_render_pass();
    setup_descriptor_set_layout();
    create_graphics_pipeline();

    create_sync_objs();
    create_command_buffer();

    create_depth_image();
    create_framebuffers();

    create_texture_image();
    create_texture_imageview();
    create_texture_sampler();

    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffer();
    create_descriptor_pool();
}

void Acvv::main_loop()
{
    while (!glfwWindowShouldClose(base_layer_))
    {
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(device_layer_);
}

void Acvv::cleanup()
{
    vkDestroyImageView(device_layer_, depth_image_, nullptr);
    vmaDestroyImage(device_layer_, depth_image_, depth_image_);
    for (auto framebuffer : swapchain_framebuffers_)
    {
        vkDestroyFramebuffer(device_layer_, framebuffer, nullptr);
    }
    swapchain_.destroy(device_layer_);

    vkDestroySampler(device_layer_, textue_sampler_, nullptr);
    vkDestroyImageView(device_layer_, texture_image_, nullptr);
    vmaDestroyImage(device_layer_, texture_image_, texture_image_);

    vkDestroyDescriptorPool(device_layer_, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(device_layer_, descriptor_set_layout_, nullptr);

    vmaDestroyBuffer(device_layer_, vertex_buffer_, vertex_buffer_);
    vmaDestroyBuffer(device_layer_, index_buffer_, index_buffer_);

    vkDestroyPipeline(device_layer_, graphics_pipeline_, nullptr);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vmaUnmapMemory(device_layer_, uniform_buffers_[i]);
        vmaDestroyBuffer(device_layer_, uniform_buffers_[i], uniform_buffers_[i]);
    }
    vkDestroyPipelineLayout(device_layer_, pipeline_Layout_, nullptr);
    vkDestroyRenderPass(device_layer_, render_pass_, nullptr);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device_layer_, get_image_semaphores_[i], nullptr);
        vkDestroySemaphore(device_layer_, image_render_semaphores_[i], nullptr);
        vkDestroyFence(device_layer_, frame_fence_[i], nullptr);
    }
    vkDestroyCommandPool(device_layer_, command_pool_, nullptr);

    device_layer_.destroy();
    base_layer_.destroy();
}
