#include "acvv.hpp"

void Acvv::setup_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings{ubo_layout_binding, sampler_layout_binding};
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 2;
    create_info.pBindings = bindings.data();

    vkCreateDescriptorSetLayout(device_, &create_info, nullptr, &descriptor_set_layout_);
}

void Acvv::create_uniform_buffer()
{
    VkDeviceSize size = sizeof(UniformBufferObject);

    uniform_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_memory_.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_map_.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        create_buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers_[i],
                      uniform_buffers_memory_[i]);

        vkMapMemory(device_, uniform_buffers_memory_[i], 0, size, 0, &uniform_buffers_map_[i]);
    }
}

void Acvv::create_descriptor_pool()
{
    std::vector<VkDescriptorPoolSize> pool_sizes(2);
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = castt(uint32_t, pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = MAX_FRAMES_IN_FLIGHT;
    vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_);

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout_);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts = layouts.data();
    descriptor_set_.resize(MAX_FRAMES_IN_FLIGHT);
    vkAllocateDescriptorSets(device_, &alloc_info, descriptor_set_.data());

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture_imageview_;
        image_info.sampler = textue_sampler_;

        std::vector<VkWriteDescriptorSet> descriptor_writes{};
        descriptor_writes.resize(castt(uint32_t, pool_sizes.size()));
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_set_[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_set_[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(device_, castt(uint32_t, descriptor_writes.size()), descriptor_writes.data(), 0,
                               nullptr);
    }
}