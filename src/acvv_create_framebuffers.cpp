#include "acvv.hpp"

void Acvv::create_framebuffers()
{
    swapchain_framebuffers_.resize(swapchain_imageviews_.size());

    for (uint32_t i = 0; i < swapchain_framebuffers_.size(); i++)
    {
        VkImageView attacgments[] = {swapchain_imageviews_[i]};

        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass_;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attacgments;
        create_info.width = swapchain_extend_.width;
        create_info.height = swapchain_extend_.height;
        create_info.layers = 1;

        vkCreateFramebuffer(device_, &create_info, nullptr, &swapchain_framebuffers_[i]);
    }
}