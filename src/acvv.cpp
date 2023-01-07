#define STB_IMAGE_IMPLEMENTATION
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
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "vk", nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_,
                                   [](GLFWwindow* window, int width, int height)
                                   {
                                       Acvv* app = reinterpret_cast<Acvv*>(glfwGetWindowUserPointer(window));
                                       while (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                                       {
                                           glfwWaitEvents();
                                       }
                                       app->reset_swapchain();
                                   });
}

void Acvv::init_vulkan()
{
    create_instance();
    setup_device();
    create_swapchain();
    get_swapchain_imageviews();

    create_render_pass();
    setup_descriptor_set_layout();
    create_graphics_pipeline();

    create_sync_objs();
    create_framebuffers();

    create_command_buffer();
    create_texture_image();

    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffer();
    create_descriptor_pool();
}

void Acvv::main_loop()
{
    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(device_);
}

void Acvv::cleanup()
{
    clear_swapchain();

    vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);

    vkDestroyBuffer(device_, vertex_buffer_, nullptr);
    vkFreeMemory(device_, vertex_buffer_memory_, nullptr);

    vkDestroyBuffer(device_, index_buffer_, nullptr);
    vkFreeMemory(device_, index_buffer_memory_, nullptr);

    vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device_, uniform_buffers_[i], nullptr);
        vkFreeMemory(device_, uniform_buffers_memory_[i], nullptr);
    }
    vkDestroyPipelineLayout(device_, pipeline_Layout_, nullptr);
    vkDestroyRenderPass(device_, render_pass_, nullptr);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device_, get_image_semaphores[i], nullptr);
        vkDestroySemaphore(device_, image_render_semaphores[i], nullptr);
        vkDestroyFence(device_, frame_fence[i], nullptr);
    }
    vkDestroyCommandPool(device_, command_pool_, nullptr);
    vkDestroyDevice(device_, nullptr);

    auto load_func = load_ext_function<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr, instance_,
                                                                            "vkDestroyDebugUtilsMessengerEXT");
    load_func(instance_, messenger_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);

    glfwDestroyWindow(window_);
    glfwTerminate();
}

uint32_t Acvv::find_memory_type(uint32_t type, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_prop{};
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_prop);

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
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer copy_command_buffer{};
    vkAllocateCommandBuffers(device_, &alloc_info, &copy_command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(copy_command_buffer, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(copy_command_buffer, src_buffer, dst_buffer, 1, &copy_region);
    vkEndCommandBuffer(copy_command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &copy_command_buffer;
    vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);

    vkFreeCommandBuffers(device_, command_pool_, 1, &copy_command_buffer);
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
    vkCreateBuffer(device_, &buffer_info, nullptr, &buffer);

    VkMemoryRequirements mem_requires{};
    vkGetBufferMemoryRequirements(device_, buffer, &mem_requires);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requires.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requires.memoryTypeBits, properties);

    vkAllocateMemory(device_, &alloc_info, nullptr, &buffer_memory);
    vkBindBufferMemory(device_, buffer, buffer_memory, 0);
}

bool check_validation_layer_support()
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

VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    auto type = [messageSeverity]()
    {
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            {
                return "WARNING";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            {
                return "ERROR";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            {
                return "INFO";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            {
                return "VERBOSE";
            }
            default:
            {
                return "UNDEFINE";
            }
        }
    };
    std::cerr << fmt::format("[Vulkan Validation Layer: {}] {}\n\n", type(), pCallbackData->pMessage);

    return VK_FALSE;
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