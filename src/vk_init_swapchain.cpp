#include "app.hpp"

#include <algorithm>

void App::setup_swapchain()
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;

    capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);
    formats = physical_device_.getSurfaceFormatsKHR(surface_);
    present_modes = physical_device_.getSurfacePresentModesKHR(surface_);

    vk::SurfaceFormatKHR choosed_format = formats[0];
    vk::PresentModeKHR choosed_present_mode = vk::PresentModeKHR::eFifo;
    vk::Extent2D choosed_extend{};

    for (const auto& avaliable_format : formats)
    {
        if (avaliable_format.format == vk::Format::eB8G8R8A8Srgb && //
            avaliable_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            choosed_format = avaliable_format;
            break;
        }
    }

    for (const auto& avaliable_present_mode : present_modes)
    {
        if (avaliable_present_mode == vk::PresentModeKHR::eMailbox)
        {
            choosed_present_mode = avaliable_present_mode;
            break;
        }
    }

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        choosed_extend = capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        choosed_extend.width = (uint32_t)width;
        choosed_extend.height = (uint32_t)height;
        choosed_extend.width = std::clamp(choosed_extend.width, //
                                          capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        choosed_extend.height = std::clamp(choosed_extend.height, //
                                           capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    {
        image_count = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR create_info{};
    create_info.surface = surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = choosed_format.format;
    create_info.imageColorSpace = choosed_format.colorSpace;
    create_info.imageExtent = choosed_extend;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    if (queue_family_indices.graphic != queue_family_indices.present)
    {
        create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = &queue_family_indices.graphic;
    }
    else
    {
        create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = choosed_present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    swapchain = device_.createSwapchainKHR(create_info);

    swapchain_image_format = choosed_format.format;
    swapchain_extend = choosed_extend;
    swapchain_images = device_.getSwapchainImagesKHR(swapchain);
}

void App::setup_swapchain_imageview()
{
    swapchain_imageviews.resize(swapchain_images.size());

    for (uint32_t i = 0; i < swapchain_imageviews.size(); i++)
    {
        vk::ImageViewCreateInfo create_info{};
        create_info.image = swapchain_images[i];
        create_info.viewType = vk::ImageViewType::e2D;
        create_info.format = swapchain_image_format;

        create_info.components.r = vk::ComponentSwizzle::eIdentity;
        create_info.components.g = vk::ComponentSwizzle::eIdentity;
        create_info.components.b = vk::ComponentSwizzle::eIdentity;
        create_info.components.a = vk::ComponentSwizzle::eIdentity;

        create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        swapchain_imageviews[i] = device_.createImageView(create_info);
    }
}

void App::clear_swapchain()
{
    for (auto framebuffer : swapchain_framebuffers)
    {
        device_.destroyFramebuffer(framebuffer);
    }
    for (const auto& imageview : swapchain_imageviews)
    {
        device_.destroyImageView(imageview);
    }
    device_.destroySwapchainKHR(swapchain);
}

void App::reset_swapchain()
{
    device_.waitIdle();

    clear_swapchain();

    setup_swapchain();
    setup_swapchain_imageview();
    setup_framebuffers();
}