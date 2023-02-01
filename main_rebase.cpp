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

struct LightPushConstants
{
    glm::vec4 position;
    glm::vec4 direction;

    glm::vec4 color;

    float strength;
    float constant;
    float linear;
    float quadratic;
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
    ccc.instance_count = 1;

    std::vector<VkExtent2D> extends(5, swapchain.extend_);
    std::vector<VkSampleCountFlagBits> samples(5, VK_SAMPLE_COUNT_1_BIT);
    std::vector<VkFormat> formats = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
                                     VK_FORMAT_D32_SFLOAT_S8_UINT};
    std::vector<VkImageUsageFlags> usages = {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //
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
    vert_shader0.create(device_layer,                                        //
                        read_file("res/shader/vert0.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);
    vert_shader1.create(device_layer,                                        //
                        read_file("res/shader/vert1.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);
    vert_shader2.create(device_layer,                                        //
                        read_file("res/shader/vert2.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_VERTEX_BIT);

    VeShaderBase frag_shader0;
    VeShaderBase frag_shader1;
    VeShaderBase frag_shader2;
    frag_shader0.create(device_layer,                                        //
                        read_file("res/shader/frag0.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader1.create(device_layer,                                        //
                        read_file("res/shader/frag1.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader2.create(device_layer,                                        //
                        read_file("res/shader/frag2.spv", std::ios::binary), //
                        "main", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipeline graphics_pipeline0;
    VkPipeline graphics_pipeline1;
    VkPipeline graphics_pipeline2;

    // first pipeline
    VkPipelineShaderStageCreateInfo shader_stages0[2]{vert_shader0, frag_shader0};

    VkPipelineVertexInputStateCreateInfo vertex_input_info0{};
    vertex_input_info0.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info0.vertexBindingDescriptionCount = binding_description.size();
    vertex_input_info0.pVertexBindingDescriptions = binding_description.data();
    vertex_input_info0.vertexAttributeDescriptionCount = attribute_description.size();
    vertex_input_info0.pVertexAttributeDescriptions = attribute_description.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = casts(uint32_t, dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachments[3]{};
    color_blend_attachments[0].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachments[1].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachments[2].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = 3;
    color_blending.pAttachments = color_blend_attachments;

    VkPipelineDepthStencilStateCreateInfo depth_sentcil{};
    depth_sentcil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_sentcil.depthTestEnable = VK_TRUE;
    depth_sentcil.depthWriteEnable = VK_TRUE;
    depth_sentcil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_sentcil.depthBoundsTestEnable = VK_FALSE;
    depth_sentcil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline0_info{};
    pipeline0_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline0_info.stageCount = 2;
    pipeline0_info.pStages = shader_stages0;
    pipeline0_info.pVertexInputState = &vertex_input_info0;
    pipeline0_info.pInputAssemblyState = &input_assembly;
    pipeline0_info.pViewportState = &viewport_state;
    pipeline0_info.pRasterizationState = &rasterizer;
    pipeline0_info.pMultisampleState = &multisample;
    pipeline0_info.pColorBlendState = &color_blending;
    pipeline0_info.pDynamicState = &dynamic_state;
    pipeline0_info.pDepthStencilState = &depth_sentcil;
    pipeline0_info.layout = pipeline_layouts[0];
    pipeline0_info.renderPass = render_pass;
    pipeline0_info.subpass = 0;
    vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline0_info, nullptr, &graphics_pipeline0);

    // second graphics pipeline
    VkPipelineShaderStageCreateInfo shader_stages1[2]{vert_shader1, frag_shader1};
    VkPipelineVertexInputStateCreateInfo vertex_input_info1{};
    vertex_input_info1.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState color_blend_attachment1{};
    color_blend_attachment1.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment1.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment1.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | //
                                             VK_COLOR_COMPONENT_G_BIT | //
                                             VK_COLOR_COMPONENT_B_BIT | //
                                             VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blend_info1{};
    color_blend_info1.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info1.attachmentCount = 1;
    color_blend_info1.pAttachments = &color_blend_attachment1;

    VkPipelineDepthStencilStateCreateInfo depth_sentcil1{};
    depth_sentcil1.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_sentcil1.depthTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline1_info{};
    pipeline1_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline1_info.stageCount = 2;
    pipeline1_info.pStages = shader_stages1;
    pipeline1_info.pVertexInputState = &vertex_input_info1;
    pipeline1_info.pInputAssemblyState = &input_assembly;
    pipeline1_info.pViewportState = &viewport_state;
    pipeline1_info.pRasterizationState = &rasterizer;
    pipeline1_info.pMultisampleState = &multisample;
    pipeline1_info.pColorBlendState = &color_blend_info1;
    pipeline1_info.pDynamicState = &dynamic_state;
    pipeline1_info.pDepthStencilState = &depth_sentcil1;
    pipeline1_info.layout = pipeline_layouts[1];
    pipeline1_info.renderPass = render_pass;
    pipeline1_info.subpass = 1;
    vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline1_info, nullptr, &graphics_pipeline1);

    // third graphics pipeline
    VkPipelineShaderStageCreateInfo shader_stages2[2]{vert_shader2, frag_shader2};
    VkPipelineVertexInputStateCreateInfo vertex_input_info2{};
    vertex_input_info2.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState color_blend_attachment2{};
    color_blend_attachment2.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | //
                                             VK_COLOR_COMPONENT_G_BIT | //
                                             VK_COLOR_COMPONENT_B_BIT | //
                                             VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blend_info2{};
    color_blend_info2.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info2.attachmentCount = 1;
    color_blend_info2.pAttachments = &color_blend_attachment2;

    VkPipelineDepthStencilStateCreateInfo depth_sentcil2{};
    depth_sentcil2.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_sentcil2.depthTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline2_info{};
    pipeline2_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline2_info.stageCount = 2;
    pipeline2_info.pStages = shader_stages2;
    pipeline2_info.pVertexInputState = &vertex_input_info2;
    pipeline2_info.pInputAssemblyState = &input_assembly;
    pipeline2_info.pViewportState = &viewport_state;
    pipeline2_info.pRasterizationState = &rasterizer;
    pipeline2_info.pMultisampleState = &multisample;
    pipeline2_info.pColorBlendState = &color_blend_info2;
    pipeline2_info.pDynamicState = &dynamic_state;
    pipeline2_info.pDepthStencilState = &depth_sentcil2;
    pipeline2_info.layout = pipeline_layouts[2];
    pipeline2_info.renderPass = render_pass;
    pipeline2_info.subpass = 2;
    vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline2_info, nullptr, &graphics_pipeline2);

    vert_shader0.destroy(device_layer);
    vert_shader1.destroy(device_layer);
    vert_shader2.destroy(device_layer);
    frag_shader0.destroy(device_layer);
    frag_shader1.destroy(device_layer);
    frag_shader2.destroy(device_layer);

    VeCommandPoolBase cmd_pool;
    cmd_pool.create(device_layer, device_layer.queue_family_indices.graphics,
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer cmd = cmd_pool.allocate_buffer(device_layer, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VeGpuSemaphore image_semaphore;
    VeGpuSemaphore submit_semaphore;
    VeGpuFence frame_fence;
    image_semaphore.create(device_layer);
    submit_semaphore.create(device_layer);
    frame_fence.create(device_layer, true);

    std::vector<VkFramebuffer> framebuffers(swapchain.image_views_.size());
    for (int i = 0; i < swapchain.image_views_.size(); i++)
    {
        VkImageView fattachments[] = {attachments[0], attachments[1], attachments[2],
                                      attachments[3], attachments[4], swapchain.image_views_[i]};
        VkFramebufferCreateInfo fcreate_info{};
        fcreate_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fcreate_info.renderPass = render_pass;
        fcreate_info.attachmentCount = 6;
        fcreate_info.pAttachments = fattachments;
        fcreate_info.width = swapchain.extend_.width;
        fcreate_info.height = swapchain.extend_.height;
        fcreate_info.layers = 1;
        vkCreateFramebuffer(device_layer, &fcreate_info, nullptr, &framebuffers[i]);
    }

    VkViewport viewport{};
    viewport.width = casts(uint32_t, swapchain.extend_.width);
    viewport.height = casts(uint32_t, swapchain.extend_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.extent = swapchain.extend_;

    for (int i = 0; i < 3; i++)
    {
        VkDescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler = VK_NULL_HANDLE;
        descriptor_image_info.imageView = attachments[i];
        descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet image_write{};
        image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        image_write.dstSet = descriptor_sets[1];
        image_write.dstBinding = i;
        image_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        image_write.descriptorCount = 1;
        image_write.pImageInfo = &descriptor_image_info;
        vkUpdateDescriptorSets(device_layer, 1, &image_write, 0, nullptr);
    }
    VkDescriptorImageInfo descriptor_image_info{};
    descriptor_image_info.sampler = VK_NULL_HANDLE;
    descriptor_image_info.imageView = attachments[3];
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet image_write{};
    image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    image_write.dstSet = descriptor_sets[2];
    image_write.dstBinding = 0;
    image_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    image_write.descriptorCount = 1;
    image_write.pImageInfo = &descriptor_image_info;
    vkUpdateDescriptorSets(device_layer, 1, &image_write, 0, nullptr);

    VeMultiThreadCmdRecorder record0;
    VeMultiThreadCmdRecorder record1;
    VeMultiThreadCmdRecorder record2;
    record0.create(device_layer);
    record1.create(device_layer);
    record2.create(device_layer);

    std::thread record_th0(std::ref(record0),
                           [&](VkCommandBuffer secondary_cmd)
                           {
                               vkCmdBindPipeline(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline0);
                               vkCmdSetViewport(secondary_cmd, 0, 1, &viewport);
                               vkCmdSetScissor(secondary_cmd, 0, 1, &scissor);
                               vkCmdBindDescriptorSets(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layouts[0], 0, 1, descriptor_sets, 0, nullptr);
                               ccc.draw(secondary_cmd);
                           });

    std::thread record_th1(std::ref(record1),
                           [&](VkCommandBuffer secondary_cmd)
                           {
                               vkCmdBindPipeline(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline1);
                               vkCmdSetViewport(secondary_cmd, 0, 1, &viewport);
                               vkCmdSetScissor(secondary_cmd, 0, 1, &scissor);
                               vkCmdBindDescriptorSets(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layouts[1], 0, 1, descriptor_sets + 1, 0, nullptr);
                               vkCmdDraw(secondary_cmd, 6, 1, 0, 0);
                           });

    std::thread record_th2(std::ref(record2),
                           [&](VkCommandBuffer secondary_cmd)
                           {
                               vkCmdBindPipeline(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline2);
                               vkCmdSetViewport(secondary_cmd, 0, 1, &viewport);
                               vkCmdSetScissor(secondary_cmd, 0, 1, &scissor);
                               vkCmdBindDescriptorSets(secondary_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layouts[2], 0, 1, descriptor_sets + 2, 0, nullptr);
                               vkCmdDraw(secondary_cmd, 6, 1, 0, 0);
                           });

    // main loop
    VeCpuTimer timer;
    while (!glfwWindowShouldClose(base_layer))
    {
        glfwPollEvents();
        ccc.update();

        auto result = vkWaitForFences(device_layer, 1, &frame_fence, VK_TRUE, UINT64_MAX);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Fence error\n");
        }
        uint32_t image_index = 0;
        vkAcquireNextImageKHR(device_layer, swapchain, UINT64_MAX, image_semaphore, VK_NULL_HANDLE, &image_index);
        vkResetFences(device_layer, 1, &frame_fence);

        ccc.instances_[0] = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f * timer.since_init_second()), {0, 1, 0});
        memcpy(ubo_map, &ubo, sizeof(ubo));

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_sets[0];
        write.dstBinding = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &buffer_info;
        vkUpdateDescriptorSets(device_layer, 1, &write, 0, nullptr);

        VkCommandBufferInheritanceInfo iifo{};
        iifo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        iifo.renderPass = render_pass;
        iifo.subpass = 0;
        iifo.framebuffer = framebuffers[image_index];
        record0.begin(iifo);
        iifo.subpass = 1;
        record1.begin(iifo);
        iifo.subpass = 2;
        record2.begin(iifo);

        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &begin_info);
        VkClearValue clear_value[6];
        clear_value[0] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[1] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[2] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[3] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[4] = {{1.0f, 0.0f}};
        clear_value[5] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass;
        render_pass_info.framebuffer = framebuffers[image_index];
        render_pass_info.renderArea.extent = swapchain.extend_;
        render_pass_info.clearValueCount = 6;
        render_pass_info.pClearValues = clear_value;

        vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        record0.wait_than_excute(cmd);

        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        record1.wait_than_excute(cmd);

        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        record2.wait_than_excute(cmd);

        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_semaphore;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &submit_semaphore;
        vkQueueSubmit(device_layer.graphics_queue_, 1, &submit_info, frame_fence);

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &submit_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &image_index;

        vkQueuePresentKHR(device_layer.present_queue_, &present_info);

        vkDeviceWaitIdle(device_layer);
    }

    record0.terminate();
    record1.terminate();
    record2.terminate();
    record_th0.join();
    record_th1.join();
    record_th2.join();
    record0.destroy(device_layer);
    record1.destroy(device_layer);
    record2.destroy(device_layer);

    vmaUnmapMemory(device_layer, uniform_buffer);
    vmaDestroyBuffer(device_layer, uniform_buffer, uniform_buffer);

    for (auto framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device_layer, framebuffer, nullptr);
    }

    cmd_pool.destroy(device_layer);
    image_semaphore.destroy(device_layer);
    submit_semaphore.destroy(device_layer);
    frame_fence.destroy(device_layer);
    vkDestroyPipeline(device_layer, graphics_pipeline0, nullptr);
    vkDestroyPipeline(device_layer, graphics_pipeline1, nullptr);
    vkDestroyPipeline(device_layer, graphics_pipeline2, nullptr);

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