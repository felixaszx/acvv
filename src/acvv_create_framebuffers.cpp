#include "acvv.hpp"

void Acvv::create_framebuffers()
{
    swapchain_framebuffers_.resize(swapchain_.image_views_.size());

    for (uint32_t i = 0; i < swapchain_framebuffers_.size(); i++)
    {
        VkImageView attacgments[] = {swapchain_.image_views_[i]};

        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass_;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attacgments;
        create_info.width = swapchain_.extend_.width;
        create_info.height = swapchain_.extend_.height;
        create_info.layers = 1;

        vkCreateFramebuffer(device_layer_, &create_info, nullptr, &swapchain_framebuffers_[i]);
    }
}