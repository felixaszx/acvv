#include <iostream>
#include <string>

#include "ve_base.hpp"
#include "ve_device.hpp"
#include "ve_image.hpp"
#include "ve_swapchain.hpp"
#include "ve_buffer.hpp"
#include "ve_cmd.hpp"
#include "ve_graphic_pipeline.hpp"

#include "glms.hpp"

int main(int argc, char** argv)
{
    VeBaseLayer base_layer(1920, 1080);
    base_layer.create("acvv");

    VeDeviceLayer device_layer;
    device_layer.create(base_layer);

    VeSwapchainBase swapchain;
    swapchain.create(base_layer, base_layer, device_layer);
    swapchain.create_image_view(device_layer);

    std::vector<VkFormat> formats = {VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT,
                                     VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    std::vector<VkExtent2D> extends(4, swapchain.extend_);
    std::vector<VkSampleCountFlagBits> samples(4, VK_SAMPLE_COUNT_1_BIT);
    std::vector<VkImageUsageFlags> usages = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
    std::vector<VkImageAspectFlags> aspects = {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
                                               VK_IMAGE_ASPECT_COLOR_BIT,
                                               VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT};
    std::vector<VeImageBase> attachments = creat_image_attachments(device_layer,     //
                                                                   formats, extends, //
                                                                   samples, usages,  //
                                                                   aspects);

    VkAttachmentDescription attachment_descriptions[5]{};
    VkAttachmentReference attachment_references0[1]{};
    VkAttachmentReference attachment_references1[4]{};
    VkAttachmentReference attachment_references2[4]{};

    for (int i = 0; i < 5; i++)
    {
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachment_descriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if (i == 3)
        {
            attachment_descriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachment_descriptions[i].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        }
        if (i == 4)
        {
            attachment_descriptions[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachment_descriptions[i].format = swapchain.image_format_;
            attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        }
    }

    attachment_references0[0].attachment = 3;
    attachment_references0[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (int i = 0; i < 4; i++)
    {
        attachment_references1[i].attachment = i;
        attachment_references1[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    attachment_references1[3].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    for (int i = 0; i < 4; i++)
    {
        attachment_references2[i].attachment = i;
        attachment_references2[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    attachment_references2[3].attachment = 4;
    attachment_references2[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpasses[3]{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].pDepthStencilAttachment = attachment_references0;

    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].colorAttachmentCount = 3;
    subpasses[1].pColorAttachments = attachment_references1;
    subpasses[1].pDepthStencilAttachment = attachment_references1 + 3;

    subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[2].inputAttachmentCount = 3;
    subpasses[2].pInputAttachments = attachment_references2;
    subpasses[2].colorAttachmentCount = 1;
    subpasses[2].pColorAttachments = attachment_references2 + 3;

    VkSubpassDependency dependencies[2]{};
    dependencies[0].srcSubpass = 0;
    dependencies[0].dstSubpass = 1;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    dependencies[1].srcSubpass = 1;
    dependencies[1].dstSubpass = 2;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.subpassCount = 3;
    render_pass_info.pSubpasses = subpasses;
    render_pass_info.attachmentCount = 5;
    render_pass_info.pAttachments = attachment_descriptions;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = dependencies;

    VkRenderPass render_pass;
    vkCreateRenderPass(device_layer, &render_pass_info, nullptr, &render_pass);

    while (!glfwWindowShouldClose(base_layer))
    {
        glfwPollEvents();
    }

    for (auto attachment : attachments)
    {
        vkDestroyImageView(device_layer, attachment, nullptr);
        vmaDestroyImage(device_layer, attachment, attachment);
    }

    vkDestroyRenderPass(device_layer, render_pass, nullptr);
    swapchain.destroy(device_layer);
    device_layer.destroy();
    base_layer.destroy();
    return EXIT_SUCCESS;
}