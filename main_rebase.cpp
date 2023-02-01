#include <iostream>
#include <string>

#define VE_ENABLE_VALIDATION
#include "ve_base.hpp"
#include "ve_device.hpp"
#include "ve_image.hpp"
#include "ve_swapchain.hpp"
#include "ve_buffer.hpp"
#include "ve_cmd.hpp"
#include "ve_graphic_pipeline.hpp"
#include "ve_mesh.hpp"

#include "glms.hpp"

struct UniformBuffer
{
    glm::mat4 view;
    glm::mat4 proj;
};

int main(int argc, char** argv)
{
    VeBaseLayer base_layer(1920, 1080);
    base_layer.create("acvv");

    VeDeviceLayer device_layer;
    device_layer.create(base_layer);

    VeSwapchainBase swapchain;
    swapchain.create(base_layer, base_layer, device_layer);
    swapchain.create_image_view(device_layer);

    VeMesh ccc("res/model/sponza/sponza.obj", 100);
    ccc.create(device_layer);

    std::vector<VkExtent2D> extends(5, swapchain.extend_);
    std::vector<VkSampleCountFlagBits> samples(5, VK_SAMPLE_COUNT_1_BIT);
    std::vector<VkFormat> formats = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_FORMAT_D32_SFLOAT_S8_UINT};
    std::vector<VkImageUsageFlags> usages = {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                                       //
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
    std::vector<VkImageAspectFlags> aspects = {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_COLOR_BIT, //
                                               VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_COLOR_BIT, //
                                               VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT};
    std::vector<VeImageBase> attachments = creat_image_attachments(device_layer,     //
                                                                   formats, extends, //
                                                                   samples, usages,  //
                                                                   aspects);
    VkAttachmentDescription attachment_descriptions[6]{};
    for (uint32_t i = 0; i < 6; i++)
    {
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attachment_descriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    attachment_descriptions[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_descriptions[4].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    attachment_descriptions[5].format = swapchain.image_format_;
    attachment_descriptions[5].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment_descriptions[5].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkAttachmentReference attachment_references0[4]{};
    VkAttachmentReference attachment_references1[4]{};
    VkAttachmentReference attachment_references2[2]{};

    // first subpass attachment
    for (uint32_t i = 0; i < 3; i++)
    {
        attachment_references0[i].attachment = i;
        attachment_references0[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    attachment_references0[3].attachment = 4;
    attachment_references0[3].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // second subpass attachment
    for (uint32_t i = 0; i < 4; i++)
    {
        attachment_references1[i].attachment = i;
        attachment_references1[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    attachment_references1[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // third subpass attachment
    attachment_references2[0].attachment = 3;
    attachment_references2[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachment_references2[1].attachment = 5;
    attachment_references2[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpasses[3]{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 3;
    subpasses[0].pColorAttachments = attachment_references0;
    subpasses[0].pDepthStencilAttachment = attachment_references0 + 3;

    subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[1].colorAttachmentCount = 1;
    subpasses[1].pColorAttachments = attachment_references1 + 3;
    subpasses[1].inputAttachmentCount = 3;
    subpasses[1].pInputAttachments = attachment_references1;

    subpasses[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[2].colorAttachmentCount = 1;
    subpasses[2].pColorAttachments = attachment_references2 + 1;
    subpasses[2].inputAttachmentCount = 1;
    subpasses[2].pInputAttachments = attachment_references2;

    VkSubpassDependency dependencies[2]{};
    dependencies[0].srcSubpass = 0;
    dependencies[0].dstSubpass = 1;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | //
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | //
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

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
    render_pass_info.attachmentCount = 6;
    render_pass_info.pAttachments = attachment_descriptions;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = dependencies;
    VkRenderPass render_pass;
    vkCreateRenderPass(device_layer, &render_pass_info, nullptr, &render_pass);

    UniformBuffer ubo{};
    ubo.view = glm::lookAt(glm::vec3(2.0f, 40.0f, 2.0f), glm::vec3(2.0f, 40.0f, 2.0f) + glm::vec3(0.0f, 0.0f, 1.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),                                       //
                                swapchain.extend_.width / (float)swapchain.extend_.height, //
                                0.1f, 1000.0f);
    ubo.proj[1][1] *= -1;
    void* ubo_map = nullptr;
    VeBufferBase uniform_buffer;
    VkBufferCreateInfo ubo_info{};
    ubo_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ubo_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ubo_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ubo_info.size = sizeof(ubo);
    VmaAllocationCreateInfo ubo_alloc_info{};
    ubo_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    ubo_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer, &ubo_info, &ubo_alloc_info, &uniform_buffer, &uniform_buffer, nullptr);
    vmaMapMemory(device_layer, uniform_buffer, &ubo_map);

    VkDescriptorSetLayout set_layouts[3]{};
    VkDescriptorSet descriptor_sets[3]{};
    VkDescriptorPool descriptor_pools[3]{};
    VkPipelineLayout pipeline_layouts[3]{};

    VkDescriptorSetLayoutBinding bindings[5]{};
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    for (int i = 1; i < 4; i++)
    {
        bindings[i].binding = i - 1;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    bindings[4].binding = 0;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo set_layout_create_infos[3]{};
    set_layout_create_infos[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_create_infos[0].bindingCount = 1;
    set_layout_create_infos[0].pBindings = bindings + 0;
    set_layout_create_infos[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_create_infos[1].bindingCount = 3;
    set_layout_create_infos[1].pBindings = bindings + 1;
    set_layout_create_infos[2].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_create_infos[2].bindingCount = 1;
    set_layout_create_infos[2].pBindings = bindings + 4;
    for (uint32_t i = 0; i < 3; i++)
    {
        vkCreateDescriptorSetLayout(device_layer, set_layout_create_infos + i, nullptr, set_layouts + i);
    }

    VkDescriptorPoolSize pool_sizes[3]{};
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[1].descriptorCount = 3;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pool_sizes[2].descriptorCount = 1;
    pool_sizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

    for (uint32_t i = 0; i < 3; i++)
    {
        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.pPoolSizes = pool_sizes + i;
        pool_create_info.poolSizeCount = 1;
        pool_create_info.maxSets = 1;
        vkCreateDescriptorPool(device_layer, &pool_create_info, nullptr, descriptor_pools + i);

        VkDescriptorSetAllocateInfo set_alloc_info{};
        set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_alloc_info.descriptorPool = descriptor_pools[i];
        set_alloc_info.descriptorSetCount = 1;
        set_alloc_info.pSetLayouts = set_layouts + i;
        vkAllocateDescriptorSets(device_layer, &set_alloc_info, descriptor_sets + i);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = set_layouts + i;
        vkCreatePipelineLayout(device_layer, &pipeline_layout_info, nullptr, pipeline_layouts + i);
    }

    auto binding_description = VeMesh::get_bindings();
    auto attribute_description = VeMesh::get_attributes();

    VeShaderBase vert_shader0;
    VeShaderBase vert_shader1;
    VeShaderBase vert_shader2;
    vert_shader0.create(device_layer,                                       //
                        read_file("res/shader/vert0.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);
    vert_shader1.create(device_layer,                                       //
                        read_file("res/shader/vert1.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);
    vert_shader2.create(device_layer,                                       //
                        read_file("res/shader/vert2.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);

    VeShaderBase frag_shader0;
    VeShaderBase frag_shader1;
    VeShaderBase frag_shader2;
    frag_shader0.create(device_layer,                                       //
                        read_file("res/shader/frag0.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader1.create(device_layer,                                       //
                        read_file("res/shader/frag1.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader2.create(device_layer,                                       //
                        read_file("res/shader/frag2.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);

    vert_shader0.destroy(device_layer);
    vert_shader1.destroy(device_layer);
    vert_shader2.destroy(device_layer);
    frag_shader0.destroy(device_layer);
    frag_shader1.destroy(device_layer);
    frag_shader2.destroy(device_layer);

    while (!glfwWindowShouldClose(base_layer))
    {
        glfwPollEvents();
    }

    vmaUnmapMemory(device_layer, uniform_buffer);
    vmaDestroyBuffer(device_layer, uniform_buffer, uniform_buffer);

    for (uint32_t i = 0; i < 3; i++)
    {
        vkDestroyDescriptorPool(device_layer, descriptor_pools[i], nullptr);
        vkDestroyDescriptorSetLayout(device_layer, set_layouts[i], nullptr);
        vkDestroyPipelineLayout(device_layer, pipeline_layouts[i], nullptr);
    }

    destroy_image_attachments(device_layer, attachments);
    vkDestroyRenderPass(device_layer, render_pass, nullptr);

    ccc.destroy(device_layer);

    swapchain.destroy(device_layer);
    device_layer.destroy();
    base_layer.destroy();
    return EXIT_SUCCESS;
}