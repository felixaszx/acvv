#ifndef VE_HPP
#define VE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <chrono>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "defines.hpp"
#include "vma.hpp"

class VeCpuTimer
{
  private:
    std::chrono::system_clock::time_point init;
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point end;

  public:
    VeCpuTimer();
    float since_init_second();
    uint32_t since_init_ms();

    void start();
    void finish();

    float get_duration_second();
    uint32_t get_duration_ms();
};

class VeMouseTracker
{
  private:
    double last_x = 0;
    double last_y = 0;
    double this_x = 0;
    double this_y = 0;

  public:
    struct VeMousePosition2D
    {
        float x = 0;
        float y = 0;
    };

    void init(GLFWwindow* window);

    void update(GLFWwindow* window);
    VeMousePosition2D get_last_position();
    VeMousePosition2D get_this_position();
    VeMousePosition2D get_delta_position();
};

#ifdef VE_ENABLE_VALIDATION
#define ATTRIB_ENABLE_VALIDATION true
#else
#define ATTRIB_ENABLE_VALIDATION false
#endif

using VeBase = MultiType<GLFWwindow*,                          //
                         VkInstance, VkDebugUtilsMessengerEXT, //
                         VkSurfaceKHR>;

struct VeBaseLayer : public VeBase
{
    const uint32_t WIDTH = 0;
    const uint32_t HEIGHT = 0;

    const bool ENABLE_VALIDATION_LAYERS = false;
    const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

    using VeBase::operator=;
    VeBaseLayer(uint32_t width, uint32_t height, bool debug = ATTRIB_ENABLE_VALIDATION);
    ~VeBaseLayer();

    void create(const std::string& name);
    void destroy();

    bool check_validation_layer_support();
    std::vector<const char*> get_required_exts();
    static VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    template <typename Func_t, typename GetFunc_t, typename... Args>
    inline Func_t load_ext_function(GetFunc_t get_func, Args... args);
};

//
// templates
//

template <typename Func_t, typename GetFunc_t, typename... Args>
inline Func_t VeBaseLayer::load_ext_function(GetFunc_t get_func, Args... args)
{
    return (Func_t)get_func(args...);
}

//
// Tools
//

std::vector<char> read_file(const std::string& file_name, std::ios_base::openmode mode);

#endif // VE_HPP
