#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fmt/core.h>

#include "vk.hpp"

inline const int WIDTH = 1200;
inline const int HEIGHT = 900;
inline const int MAX_FRAMES_IN_FLIGHT = 2;

inline const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
inline const std::vector<const char*> DEVICE_EXTS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#define VK_CHECK(x)                                                       \
    try                                                                   \
    {                                                                     \
        x;                                                                \
    }                                                                     \
    catch (vk::SystemError err)                                           \
    {                                                                     \
        std::cerr << "[Vulkan System Error] " << err.what() << std::endl; \
        throw;                                                            \
    }

#ifdef NDEBUG
inline const bool ENABLE_VALIDATION_LAYERS = false;
#else
inline const bool ENABLE_VALIDATION_LAYERS = true;
#endif

class App
{
  private:
    GLFWwindow* window = nullptr;

    vk::Instance instance_{};
    vk::DebugUtilsMessengerEXT debug_messenger_{};
    vk::SurfaceKHR surface_{};

    vk::PhysicalDevice physical_device_{};
    vk::Device device_{};

    std::vector<uint32_t> queue_families_indices{};
    vk::Queue graphics_queue_{};
    vk::Queue present_queue_{};

    vk::SwapchainKHR swapchain;
    vk::Format swapchain_image_format;
    vk::Extent2D swapchain_extend;
    std::vector<vk::Image> swapchain_images;
    std::vector<vk::ImageView> swapchain_imageviews;

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
};

#endif // APP_HPP
