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
    create_framebuffers();

    create_command_buffer();

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
    for (auto framebuffer : swapchain_framebuffers_)
    {
        vkDestroyFramebuffer(device_layer_, framebuffer, nullptr);
    }
    swapchain_.destroy(device_layer_);

    vkDestroySampler(device_layer_, textue_sampler_, nullptr);
    vkDestroyImageView(device_layer_, texture_imageview_, nullptr);
    vkDestroyImage(device_layer_, texture_image_, nullptr);
    vkFreeMemory(device_layer_, texture_image_memory_, nullptr);

    vkDestroyDescriptorPool(device_layer_, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(device_layer_, descriptor_set_layout_, nullptr);

    vkDestroyBuffer(device_layer_, vertex_buffer_, nullptr);
    vkFreeMemory(device_layer_, vertex_buffer_memory_, nullptr);

    vkDestroyBuffer(device_layer_, index_buffer_, nullptr);
    vkFreeMemory(device_layer_, index_buffer_memory_, nullptr);

    vkDestroyPipeline(device_layer_, graphics_pipeline_, nullptr);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device_layer_, uniform_buffers_[i], nullptr);
        vkFreeMemory(device_layer_, uniform_buffers_memory_[i], nullptr);
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

uint32_t Acvv::find_memory_type(uint32_t type, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_prop{};
    vkGetPhysicalDeviceMemoryProperties(device_layer_, &mem_prop);

    for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++)
    {
        if ((type & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type\n");
}

void Acvv::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer_);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(cmd, src_buffer, dst_buffer, 1, &copy_region);

    cmd.end(device_layer_);
}

void Acvv::create_buffer(VkDeviceSize size,                                          //
                         VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, //
                         VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device_layer_, &buffer_info, nullptr, &buffer);

    VkMemoryRequirements mem_requires{};
    vkGetBufferMemoryRequirements(device_layer_, buffer, &mem_requires);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requires.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requires.memoryTypeBits, properties);

    vkAllocateMemory(device_layer_, &alloc_info, nullptr, &buffer_memory);
    vkBindBufferMemory(device_layer_, buffer, buffer_memory, 0);
}