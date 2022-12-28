#include "app.hpp"

void App::setup_framebuffers()
{
    swapchain_framebuffers.resize(swapchain_imageviews.size());

    for (uint32_t i = 0; i < swapchain_imageviews.size(); i++)
    {
        vk::ImageView attachments[] = {swapchain_imageviews[i]};

        vk::FramebufferCreateInfo create_info{};
        create_info.renderPass = render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attachments;
        create_info.width = swapchain_extend.width;
        create_info.height = swapchain_extend.height;
        create_info.layers = 1;

       swapchain_framebuffers[i] = device_.createFramebuffer(create_info);
    }
}
