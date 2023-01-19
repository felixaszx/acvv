#ifndef VE_SWAPCHAIN_HPP
#define VE_SWAPCHAIN_HPP

#include "ve_device.hpp"

class VeSwapchainBase
{
  private:
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;

  public:
    operator VkSwapchainKHR();
    VkSwapchainKHR* operator&();

    VkFormat image_format_{};
    VkExtent2D extend_{};
    std::vector<VkImage> images_{};
    std::vector<VkImageView> image_views_{};

    void create(GLFWwindow* window, VkSurfaceKHR surface, VeDeviceLayer ve_device);
    void destroy(VkDevice ve_device);

    void create_image_view(VkDevice device);
};

#endif // VE_SWAPCHAIN_HPP
