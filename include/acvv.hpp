#ifndef ACVV_HPP
#define ACVV_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

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

    QueueFamilyIndex queue_family_indices{};
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchain_image_format_{};
    VkExtent2D swapchain_extend_{};
    std::vector<VkImage> swapchain_images_{};
    std::vector<VkImageView> swapchain_imageviews_{};

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

    uint32_t current_frame = 0;
    std::vector<VkSemaphore> get_image_semaphores{};
    std::vector<VkSemaphore> image_render_semaphores{};
    std::vector<VkFence> frame_fence{};

  public:
    void run();
    void init_window();
    void init_vulkan();
    void main_loop();
    void cleanup();

    template <typename Func_t, typename GetFunc_t, typename... Args>
    Func_t load_ext_function(GetFunc_t get_func, Args... args);
    void create_instance();

    void setup_physical_device();

    uint32_t find_memory_type(uint32_t type, VkMemoryPropertyFlags properties);
    void create_buffer(VkDeviceSize size,                                          //
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, //
                       VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
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

#endif // ACVV_HPP
