#ifndef ACVV_HPP
#define ACVV_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <functional>
#include <fstream>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <stb/stb_image.h>

#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma.hpp"

#include "vkm.hpp"

inline const int WIDTH = 1200;
inline const int HEIGHT = 900;
inline const int MAX_FRAMES_IN_FLIGHT = 2;

inline const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
inline const std::vector<const char*> REQUIRED_DEVICE_EXTS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
inline const bool ENABLE_VALIDATION_LAYERS = false;
#else
inline const bool ENABLE_VALIDATION_LAYERS = true;
#endif

#define castt(type, var) static_cast<type>(var)

struct QueueFamilyIndex
{
    uint32_t graphics;
    uint32_t present;
};

class Acvv
{
  private:
    GLFWwindow* window_ = nullptr;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VmaAllocator vma_allocator_ = VK_NULL_HANDLE;

    QueueFamilyIndex queue_family_indices{};
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchain_image_format_{};
    VkExtent2D swapchain_extend_{};
    std::vector<VkImage> swapchain_images_{};
    std::vector<VkImageView> swapchain_imageviews_{};

    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_Layout_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapchain_framebuffers_{};

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_{};

    VkBuffer vertex_buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory_ = VK_NULL_HANDLE;
    VkBuffer index_buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory index_buffer_memory_ = VK_NULL_HANDLE;

    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_set_{};

    std::vector<VkBuffer> uniform_buffers_{};
    std::vector<VkDeviceMemory> uniform_buffers_memory_{};
    std::vector<void*> uniform_buffers_map_{};

    uint32_t current_frame_ = 0;
    std::vector<VkSemaphore> get_image_semaphores_{};
    std::vector<VkSemaphore> image_render_semaphores_{};
    std::vector<VkFence> frame_fence_{};

    VkImage texture_image_ = VK_NULL_HANDLE;
    VkDeviceMemory texture_image_memory_ = VK_NULL_HANDLE;

  public:
    void run();
    void init_window();
    void init_vulkan();
    void main_loop();
    void cleanup();

    template <typename Func_t, typename GetFunc_t, typename... Args>
    Func_t load_ext_function(GetFunc_t get_func, Args... args);
    void create_instance();

    void setup_device();

    void create_swapchain();
    void clear_swapchain();
    void reset_swapchain();
    void get_swapchain_imageviews();

    VkShaderModule create_shader_module(const std::vector<char>& code);
    void create_render_pass();
    void setup_descriptor_set_layout();
    void create_graphics_pipeline();

    void create_sync_objs();
    void create_framebuffers();

    void create_command_buffer();

    void create_texture_image();
    void create_vertex_buffer();
    void create_index_buffer();
    void create_uniform_buffer();
    void create_descriptor_pool();

    void record_command(VkCommandBuffer command_buffer, uint32_t image_index);
    void update_ubo(uint32_t current_image);
    void draw_frame();

    uint32_t find_memory_type(uint32_t type, VkMemoryPropertyFlags properties);
    void create_buffer(VkDeviceSize size,                                          //
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, //
                       VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    VkCommandBuffer begin_single_commandbuffer();
    void end_single_commandbuffer(VkCommandBuffer commandbuffer);

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};

bool check_validation_layer_support();
std::vector<const char*> get_required_exts();
VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,    //
                                        VkDebugUtilsMessageTypeFlagsEXT messageType,               //
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, //
                                        void* pUserData);

template <typename Func_t, typename GetFunc_t, typename... Args>
inline Func_t Acvv::load_ext_function(GetFunc_t get_func, Args... args)
{
    return (Func_t)get_func(args...);
}

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

    static VkVertexInputBindingDescription get_input_binding_description()
    {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions;

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }
};

const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, //
                                      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  //
                                      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   //
                                      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

#endif // ACVV_HPP
