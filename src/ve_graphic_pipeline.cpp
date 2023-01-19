#include "ve_graphic_pipeline.hpp"

void VeShaderBase::create(VkDevice device, const std::vector<char>& shader_code, const std::string& entry_name,
                          VkShaderStageFlagBits stage)
{
    VkShaderModuleCreateInfo module_info{};
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize = casts(uint32_t, shader_code.size());
    module_info.pCode = castr(const uint32_t*, shader_code.data());
    vkCreateShaderModule(device, &module_info, nullptr, ptr());

    auto& stage_info = data<VkPipelineShaderStageCreateInfo>();
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.stage = stage;
    stage_info.module = *this;
    stage_info.pName = entry_name.data();
}

void VeShaderBase::destroy(VkDevice device)
{
    vkDestroyShaderModule(device, *this, nullptr);
}

std::vector<VeImageBase> creat_image_attachments(VeDeviceLayer device_layer, const std::vector<VkFormat>& formats, //
                                                 const std::vector<VkExtent2D>& extends,                           //
                                                 const std::vector<VkSampleCountFlagBits>& samples,                //
                                                 const std::vector<VkImageUsageFlags>& usages,                     //
                                                 const std::vector<VkImageAspectFlags>& aspects)
{
    std::vector<VeImageBase> attachments;
    attachments.resize(formats.size());

    for (uint32_t i = 0; i < attachments.size(); i++)
    {
        VkImageCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.extent.width = extends[i].width;
        create_info.extent.height = extends[i].height;
        create_info.extent.depth = 1;
        create_info.mipLevels = 1;
        create_info.arrayLayers = 1;
        create_info.format = formats[i];
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        create_info.usage = usages[i];
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.samples = samples[i];
        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vmaCreateImage(device_layer, &create_info, &alloc_info, &attachments[i], &attachments[i], nullptr);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = attachments[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = formats[i];
        view_info.subresourceRange.aspectMask = aspects[i];
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        vkCreateImageView(device_layer, &view_info, nullptr, &attachments[i]);
    }

    return attachments;
}
