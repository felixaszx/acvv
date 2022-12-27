#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

inline const int WIDTH = 1200;
inline const int HEIGHT = 900;
inline const int MAX_FRAMES_IN_FLIGHT = 2;

inline const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
inline const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
inline const bool enableValidationLayers = false;
#else
inline const bool enableValidationLayers = true;
#endif

class App
{
  private:
    GLFWwindow* window;

    vk::Instance instance_;

  public:
    void run();
    void init_window();
    void init_vulkan();
    void main_loop();
    void cleanup();

    void create_instance();
};

#endif // APP_HPP
