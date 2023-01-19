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
    VkAttachmentReference attachment_references0[4]{};
    VkAttachmentReference attachment_references1[5]{};

    for (int i = 0; i < 5; i++)
    {
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].format = VK_FORMAT_R16G16B16_SFLOAT;
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

    while (!glfwWindowShouldClose(base_layer))
    {
        glfwPollEvents();
    }

    for (auto attachment : attachments)
    {
        vkDestroyImageView(device_layer, attachment, nullptr);
        vmaDestroyImage(device_layer, attachment, attachment);
    }

    swapchain.destroy(device_layer);
    device_layer.destroy();
    base_layer.destroy();
    return EXIT_SUCCESS;
}