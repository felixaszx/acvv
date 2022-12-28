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
    void setup_sync_objs();

    void record_command(vk::CommandBuffer command_buffer, uint32_t image_index);
    void draw_frame();
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
#endif // APP_HPP
