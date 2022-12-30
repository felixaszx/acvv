#include "app.hpp"

void App::setup_descriptor_set_layout()
{
    vk::DescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutCreateInfo layout_info{};
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    descriptor_set_layout = device_.createDescriptorSetLayout(layout_info);
}

void App::setup_uniform_buffer()
{
    vk::DeviceSize size = sizeof(UniformBufferObject);

    uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_map.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        create_buffer(size, vk::BufferUsageFlagBits::eUniformBuffer, //
                      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                      uniform_buffers[i], uniform_buffers_memory[i]);

        uniform_buffers_map[i] = device_.mapMemory(uniform_buffers_memory[i], 0, size);
    }
}

void App::setup_descriptor_pool()
{
    vk::DescriptorPoolSize pool_size{};
    pool_size.descriptorCount = MAX_FRAMES_IN_FLIGHT;

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = MAX_FRAMES_IN_FLIGHT;
    descriptor_pool = device_.createDescriptorPool(pool_info);

    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout);
    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts = layouts.data();
    descriptor_set = device_.allocateDescriptorSets(alloc_info);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        vk::WriteDescriptorSet descriptor_write{};
        descriptor_write.dstSet = descriptor_set[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        device_.updateDescriptorSets(descriptor_write, {});
    }
}