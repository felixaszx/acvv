#include "acvv.hpp"

void Acvv::create_swapchain()
{
    uint32_t surface_formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &surface_formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &surface_formats_count, surface_formats.data());

    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_modes_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_modes_count, present_modes.data());

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    VkSurfaceFormatKHR selected_format = surface_formats[0];
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const VkSurfaceFormatKHR& format : surface_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && //
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            selected_format = format;
            break;
        }
    }

    for (const VkPresentModeKHR& present_mode : present_modes)
    {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            selected_present_mode = present_mode;
            break;
        }
    }

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        swapchain_extend_ = capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);
        swapchain_extend_.width = castt(uint32_t, width);
        swapchain_extend_.height = castt(uint32_t, height);
        swapchain_extend_.width = std::clamp(swapchain_extend_.width, //
                                             capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchain_extend_.height = std::clamp(swapchain_extend_.height, //
                                              capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t image_count = (capabilities.minImageCount + 1 > capabilities.maxImageCount)
                               ? capabilities.maxImageCount
                               : capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = surface_;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = selected_format.format;
    swapchain_create_info.imageColorSpace = selected_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extend_;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queue_family_indices.graphics != queue_family_indices.present)
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = &queue_family_indices.graphics;
    }
    else
    {
        swapchain_create_info.queueFamilyIndexCount = 0;
    }

    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(device_, &swapchain_create_info, nullptr, &swapchain_);
    swapchain_image_format_ = selected_format.format;
    swapchain_images_.reserve(image_count);
    vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());
}

void Acvv::get_swapchain_imageviews()
{
    swapchain_imageviews_.resize(swapchain_images_.size());

    for (uint32_t i = 0; i < swapchain_imageviews_.size(); i++)
    {
        VkImageViewCreateInfo imageview_create_info{};
        imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageview_create_info.image = swapchain_images_[i];
        imageview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageview_create_info.format = swapchain_image_format_;

        imageview_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageview_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageview_create_info.subresourceRange.baseMipLevel = 0;
        imageview_create_info.subresourceRange.levelCount = 1;
        imageview_create_info.subresourceRange.baseArrayLayer = 0;
        imageview_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(device_, &imageview_create_info, nullptr, &swapchain_imageviews_[i]);
    }
}

void Acvv::clear_swapchain()
{
    for (VkFramebuffer& framebuffer : swapchain_framebuffers_)
    {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
    }
    for (VkImageView& imageview : swapchain_imageviews_)
    {
        vkDestroyImageView(device_, imageview, nullptr);
    }
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

void Acvv::reset_swapchain()
{
    vkDeviceWaitIdle(device_);

    clear_swapchain();

    create_swapchain();
    get_swapchain_imageviews();
    create_framebuffers();
}