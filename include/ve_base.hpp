#ifndef VE_HPP
#define VE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <fstream>
#include <chrono>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <fmt/core.h>

#include "defines.hpp"
#include "vma.hpp"

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
    VeBaseLayer(uint32_t width, uint32_t height,
#ifdef NDEBUG
            bool debug = false
#else
            bool debug = true
#endif
    );
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

#endif // VE_HPP
