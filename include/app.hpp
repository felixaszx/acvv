#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fmt/core.h>

#include "vk.hpp"

inline const int WIDTH = 1200;
inline const int HEIGHT = 900;
inline const int MAX_FRAMES_IN_FLIGHT = 2;

inline const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
inline const std::vector<const char*> DEVICE_EXTS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
inline const bool ENABLE_VALIDATION_LAYERS = false;
#else
inline const bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct QueueFamilyIndex
{
    uint32_t graphic;
    uint32_t present;
};

class App
{
  private:
    GLFWwindow* window{};

    vk::Instance instance_{};
    vk::DebugUtilsMessengerEXT debug_messenger_{};
    vk::SurfaceKHR surface_{};

    vk::PhysicalDevice physical_device_{};
    vk::Device device_{};

    QueueFamilyIndex queue_family_indices{};
    vk::Queue graphics_queue_{};
    vk::Queue present_queue_{};

    vk::SwapchainKHR swapchain{};
    vk::Format swapchain_image_format{};
    vk::Extent2D swapchain_extend{};
    std::vector<vk::Image> swapchain_images{};
    std::vector<vk::ImageView> swapchain_imageviews{};

    vk::RenderPass render_pass{};
    vk::PipelineLayout pipeline_layout{};
    vk::Pipeline graphics_pipeline{};
    std::vector<vk::Framebuffer> swapchain_framebuffers{};

    vk::Buffer vertex_buffer{};
    vk::DeviceMemory vertex_buffer_memory{};
    vk::CommandPool command_pool{};
    std::vector<vk::CommandBuffer> command_buffers{};

    uint32_t current_frame = 0;
    std::vector<vk::Semaphore> image_semaphores{};
    std::vector<vk::Semaphore> render_semaphores{};
    std::vector<vk::Fence> in_flights{};

  public:
    void run();
    void init_window();
    void init_vulkan();
    void main_loop();
    void cleanup();

    void create_instance();
    void setup_physical_device();

    void setup_swapchain();
    void setup_swapchain_imageview();
    void clear_swapchain();
    void reset_swapchain();

    vk::ShaderModule create_shader_module(const std::vector<char>& code);
    void setup_render_pass();
    void setup_graphic_pipeline();

    void setup_framebuffers();
    void setup_command_buffer();
    void setup_vertex_buffer();
    void setup_sync_objs();

    void record_command(vk::CommandBuffer command_buffer, uint32_t image_index);
    void draw_frame();

    uint32_t find_memory_type(uint32_t type, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties mem_prop = physical_device_.getMemoryProperties();

        for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++)
        {
            if ((type & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type\n");
    }

    void create_buffer(vk::DeviceSize size,                                            //
                       vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, //
                       vk::Buffer& buffer, vk::DeviceMemory& buffer_memory)
    {
        vk::BufferCreateInfo buffer_info{};
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = vk::SharingMode::eExclusive;
        buffer = device_.createBuffer(buffer_info);

        vk::MemoryRequirements mem_requires = device_.getBufferMemoryRequirements(buffer);

        vk::MemoryAllocateInfo alloc_info{};
        alloc_info.allocationSize = mem_requires.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requires.memoryTypeBits, properties);
        buffer_memory = device_.allocateMemory(alloc_info);
        device_.bindBufferMemory(buffer, buffer_memory, 0);
    }

    void copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size)
    {
        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.level = vk::CommandBufferLevel::ePrimary;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        vk::CommandBuffer copy_command_buffer{};
        copy_command_buffer = device_.allocateCommandBuffers(alloc_info)[0];

        vk::CommandBufferBeginInfo begin_info{};
        begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        copy_command_buffer.begin(begin_info);

        vk::BufferCopy copy_region{};
        copy_region.size = size;
        copy_command_buffer.copyBuffer(src_buffer, dst_buffer, copy_region);

        copy_command_buffer.end();

        vk::SubmitInfo submit_info{};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &copy_command_buffer;
        graphics_queue_.submit(submit_info);
        graphics_queue_.waitIdle();

        device_.freeCommandBuffers(command_pool, copy_command_buffer);
    }
};

inline std::vector<char> read_file(const std::string& file_name, std::ios_base::openmode mode)
{

    std::ifstream file(file_name, std::ios::ate | mode);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file\n");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription get_input_binding_description()
    {
        vk::VertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = vk::VertexInputRate::eVertex;

        return binding_description;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> get_attribute_descriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 2> attribute_descriptions;

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = vk::Format::eR32G32Sfloat;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }
};

const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}}, //
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},  //
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

#endif // APP_HPP
