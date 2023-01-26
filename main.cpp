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

#include "glms.hpp"

#include "tol.hpp"

struct UniformBuffer
{
    glm::mat4 model;
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

    std::vector<VkFormat> formats = {VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT,
                                     VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    std::vector<VkExtent2D> extends(4, swapchain.extend_);
    std::vector<VkSampleCountFlagBits> samples(4, VK_SAMPLE_COUNT_1_BIT);
    std::vector<VkImageUsageFlags> usages = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
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

    UniformBuffer ubo{};
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),                                       //
                                swapchain.extend_.width / (float)swapchain.extend_.height, //
                                0.1f, 10.0f);
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
    memcpy(ubo_map, &ubo, sizeof(ubo));

    VkDescriptorSetLayout set_layouts[2]{};
    VkDescriptorSet descriptor_sets[2]{};
    VkDescriptorPool descriptor_pools[2]{};

    VkPipelineLayout pipeline_layouts[2]{};
    VkPipeline graphics_pipelines[3]{};

    auto descriptor0 = [&]()
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo set_layout_create_info{};
        set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        set_layout_create_info.bindingCount = 1;
        set_layout_create_info.pBindings = &binding;
        vkCreateDescriptorSetLayout(device_layer, &set_layout_create_info, nullptr, set_layouts + 0);

        VkDescriptorPoolSize pool_size;
        pool_size.descriptorCount = 1;
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.pPoolSizes = &pool_size;
        pool_create_info.poolSizeCount = 1;
        pool_create_info.maxSets = 1;
        vkCreateDescriptorPool(device_layer, &pool_create_info, nullptr, descriptor_pools + 0);

        VkDescriptorSetAllocateInfo set_alloc_info{};
        set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_alloc_info.descriptorPool = descriptor_pools[0];
        set_alloc_info.descriptorSetCount = 1;
        set_alloc_info.pSetLayouts = set_layouts + 0;
        vkAllocateDescriptorSets(device_layer, &set_alloc_info, descriptor_sets + 0);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = set_layouts + 0;
        vkCreatePipelineLayout(device_layer, &pipeline_layout_info, nullptr, pipeline_layouts + 0);
    };
    auto descriptor1 = [&]()
    {
        VkDescriptorSetLayoutBinding bindings[3]{};
        for (int i = 0; i < 3; i++)
        {
            bindings[i].binding = i;
            bindings[i].descriptorCount = 1;
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo set_layout_create_info{};
        set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        set_layout_create_info.bindingCount = 3;
        set_layout_create_info.pBindings = bindings;
        vkCreateDescriptorSetLayout(device_layer, &set_layout_create_info, nullptr, set_layouts + 1);

        VkDescriptorPoolSize pool_size;
        pool_size.descriptorCount = 3;
        pool_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.pPoolSizes = &pool_size;
        pool_create_info.poolSizeCount = 1;
        pool_create_info.maxSets = 1;
        vkCreateDescriptorPool(device_layer, &pool_create_info, nullptr, descriptor_pools + 1);

        VkDescriptorSetAllocateInfo set_alloc_info{};
        set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_alloc_info.descriptorPool = descriptor_pools[1];
        set_alloc_info.descriptorSetCount = 1;
        set_alloc_info.pSetLayouts = set_layouts + 1;
        vkAllocateDescriptorSets(device_layer, &set_alloc_info, descriptor_sets + 1);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = set_layouts + 1;
        vkCreatePipelineLayout(device_layer, &pipeline_layout_info, nullptr, pipeline_layouts + 1);
    };

    descriptor0();
    descriptor1();

    auto pipeline0 = [&]()
    {
        // pipeline
        auto vert_shader_code = read_file("res/shader/vert.spv", std::ios::binary);
        VeShaderBase vert_shader;
        vert_shader.create(device_layer, vert_shader_code, "main", VK_SHADER_STAGE_VERTEX_BIT);
        VkPipelineShaderStageCreateInfo shader_stages[1]{vert_shader};

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        VkPipelineDepthStencilStateCreateInfo depth_sentcil{};
        depth_sentcil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_sentcil.depthTestEnable = VK_TRUE;
        depth_sentcil.depthWriteEnable = VK_TRUE;
        depth_sentcil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_sentcil.depthBoundsTestEnable = VK_FALSE;
        depth_sentcil.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipeline0_info{};
        pipeline0_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline0_info.stageCount = 1;
        pipeline0_info.pStages = shader_stages;
        pipeline0_info.pVertexInputState = &vertex_input_info;
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

        vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline0_info, nullptr, graphics_pipelines + 0);

        vert_shader.destroy(device_layer);
    };
    auto pipeline1 = [&]()
    {
        auto vert_shader_code = read_file("res/shader/vert.spv", std::ios::binary);
        auto frag_shader_code = read_file("res/shader/frag.spv", std::ios::binary);
        VeShaderBase vert_shader;
        VeShaderBase frag_shader;
        vert_shader.create(device_layer, vert_shader_code, "main", VK_SHADER_STAGE_VERTEX_BIT);
        frag_shader.create(device_layer, frag_shader_code, "main", VK_SHADER_STAGE_FRAGMENT_BIT);
        VkPipelineShaderStageCreateInfo shader_stages[2]{vert_shader, frag_shader};

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
        depth_sentcil.depthWriteEnable = VK_FALSE;
        depth_sentcil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_sentcil.depthBoundsTestEnable = VK_FALSE;
        depth_sentcil.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipeline1_info{};
        pipeline1_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline1_info.stageCount = 2;
        pipeline1_info.pStages = shader_stages;
        pipeline1_info.pVertexInputState = &vertex_input_info;
        pipeline1_info.pInputAssemblyState = &input_assembly;
        pipeline1_info.pViewportState = &viewport_state;
        pipeline1_info.pRasterizationState = &rasterizer;
        pipeline1_info.pMultisampleState = &multisample;
        pipeline1_info.pColorBlendState = &color_blending;
        pipeline1_info.pDynamicState = &dynamic_state;
        pipeline1_info.pDepthStencilState = &depth_sentcil;
        pipeline1_info.layout = pipeline_layouts[0];
        pipeline1_info.renderPass = render_pass;
        pipeline1_info.subpass = 1;
        vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline1_info, nullptr, graphics_pipelines + 1);

        vert_shader.destroy(device_layer);
        frag_shader.destroy(device_layer);
    };
    auto pipeline2 = [&]()
    {
        auto vert_shader_code = read_file("res/shader/vert2.spv", std::ios::binary);
        auto frag_shader_code = read_file("res/shader/frag2.spv", std::ios::binary);
        VeShaderBase vert_shader;
        VeShaderBase frag_shader;
        vert_shader.create(device_layer, vert_shader_code, "main", VK_SHADER_STAGE_VERTEX_BIT);
        frag_shader.create(device_layer, frag_shader_code, "main", VK_SHADER_STAGE_FRAGMENT_BIT);
        VkPipelineShaderStageCreateInfo shader_stages[2]{vert_shader, frag_shader};

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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

        VkPipelineColorBlendAttachmentState color_blend_attachments[1]{};
        color_blend_attachments[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = color_blend_attachments;

        VkPipelineDepthStencilStateCreateInfo depth_sentcil{};
        depth_sentcil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        VkGraphicsPipelineCreateInfo pipeline2_info{};
        pipeline2_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline2_info.stageCount = 2;
        pipeline2_info.pStages = shader_stages;
        pipeline2_info.pVertexInputState = &vertex_input_info;
        pipeline2_info.pInputAssemblyState = &input_assembly;
        pipeline2_info.pViewportState = &viewport_state;
        pipeline2_info.pRasterizationState = &rasterizer;
        pipeline2_info.pMultisampleState = &multisample;
        pipeline2_info.pColorBlendState = &color_blending;
        pipeline2_info.pDynamicState = &dynamic_state;
        pipeline2_info.pDepthStencilState = &depth_sentcil;
        pipeline2_info.layout = pipeline_layouts[1];
        pipeline2_info.renderPass = render_pass;
        pipeline2_info.subpass = 2;
        vkCreateGraphicsPipelines(device_layer, VK_NULL_HANDLE, 1, &pipeline2_info, nullptr, graphics_pipelines + 2);

        vert_shader.destroy(device_layer);
        frag_shader.destroy(device_layer);
    };

    pipeline0();
    pipeline1();
    pipeline2();

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

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = device_layer.queue_family_indices.graphics;
    VkCommandPool command_pool;
    vkCreateCommandPool(device_layer, &pool_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd_alloc_info{};
    cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.commandPool = command_pool;
    cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc_info.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device_layer, &cmd_alloc_info, &cmd);

    VkSemaphore image_semaphore;
    VkSemaphore submit_semaphore;
    VkFence frame_fence;

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateSemaphore(device_layer, &semaphore_info, nullptr, &image_semaphore);
    vkCreateSemaphore(device_layer, &semaphore_info, nullptr, &submit_semaphore);
    vkCreateFence(device_layer, &fence_info, nullptr, &frame_fence);

    auto start = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(base_layer))
    {
        glfwPollEvents();

        auto curr = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(curr - start).count();
        ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * time), {0, 0, 1});
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

        if (vkWaitForFences(device_layer, 1, &frame_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        {
            throw std::runtime_error("Fence error\n");
        }
        uint32_t image_index = 0;
        vkAcquireNextImageKHR(device_layer, swapchain, UINT64_MAX, image_semaphore, VK_NULL_HANDLE, &image_index);
        vkResetFences(device_layer, 1, &frame_fence);

        VkImageView fattachments[] = {attachments[0], attachments[1], attachments[2], attachments[3],
                                      swapchain.image_views_[image_index]};
        VkFramebufferCreateInfo fcreate_info{};
        fcreate_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fcreate_info.renderPass = render_pass;
        fcreate_info.attachmentCount = 5;
        fcreate_info.pAttachments = fattachments;
        fcreate_info.width = swapchain.extend_.width;
        fcreate_info.height = swapchain.extend_.height;
        fcreate_info.layers = 1;
        VkFramebuffer framebuffer;
        vkCreateFramebuffer(device_layer, &fcreate_info, nullptr, &framebuffer);

        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &begin_info);
        VkClearValue clear_value[5];
        clear_value[0] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[1] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[2] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_value[3] = {{1.0f, 0.0f}};
        clear_value[4] = {{0.0f, 0.0f, 0.0f, 1.0f}};
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass;
        render_pass_info.framebuffer = framebuffer;
        render_pass_info.renderArea.extent = swapchain.extend_;
        render_pass_info.clearValueCount = 5;
        render_pass_info.pClearValues = clear_value;
        vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipelines[0]);
        VkViewport viewport{};
        viewport.width = casts(uint32_t, swapchain.extend_.width);
        viewport.height = casts(uint32_t, swapchain.extend_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.extent = swapchain.extend_;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[0], 0, 1, descriptor_sets, 0,
                                nullptr);
        vkCmdDraw(cmd, 12, 1, 0, 0);

        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipelines[1]);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[0], 0, 1, descriptor_sets, 0,
                                nullptr);
        vkCmdDraw(cmd, 12, 1, 0, 0);

        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipelines[2]);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[1], 0, 1, descriptor_sets + 1, 0,
                                nullptr);
        vkCmdDraw(cmd, 6, 1, 0, 0);

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
        vkDestroyFramebuffer(device_layer, framebuffer, nullptr);
    }

    for (auto attachment : attachments)
    {
        vkDestroyImageView(device_layer, attachment, nullptr);
        vmaDestroyImage(device_layer, attachment, attachment);
    }

    vkDestroyFence(device_layer, frame_fence, nullptr);
    vkDestroySemaphore(device_layer, image_semaphore, nullptr);
    vkDestroySemaphore(device_layer, submit_semaphore, nullptr);

    vkFreeCommandBuffers(device_layer, command_pool, 1, &cmd);
    vkDestroyCommandPool(device_layer, command_pool, nullptr);

    vmaUnmapMemory(device_layer, uniform_buffer);
    vmaDestroyBuffer(device_layer, uniform_buffer, uniform_buffer);
    for (int i = 0; i < 2; i++)
    {
        vkDestroyDescriptorPool(device_layer, descriptor_pools[i], nullptr);
        vkDestroyDescriptorSetLayout(device_layer, set_layouts[i], nullptr);
        vkDestroyPipelineLayout(device_layer, pipeline_layouts[i], nullptr);
    }

    for (int i = 0; i < 3; i++)
    {
        vkDestroyPipeline(device_layer, graphics_pipelines[i], nullptr);
    }

    vkDestroyRenderPass(device_layer, render_pass, nullptr);
    swapchain.destroy(device_layer);
    device_layer.destroy();
    base_layer.destroy();
    return EXIT_SUCCESS;
}