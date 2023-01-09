#include "acvv.hpp"

void Acvv::record_command(VkCommandBuffer command_buffer, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkClearValue clear_color({0.0f, 0.0f, 0.0f, 1.0f});
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = swapchain_framebuffers_[image_index];
    render_pass_info.renderArea.extent = swapchain_extend_;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
    VkBuffer vertex_buffers[] = {vertex_buffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport{};
    viewport.width = castt(uint32_t, swapchain_extend_.width);
    viewport.height = castt(uint32_t, swapchain_extend_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = swapchain_extend_;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_Layout_, 0, 1,
                            &descriptor_set_[current_frame_], 0, nullptr);
    vkCmdDrawIndexed(command_buffer, castt(uint32_t, indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void Acvv::update_ubo(uint32_t current_image)
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    UniformBufferObject ubo{};
    ubo.model = vkm::rotate(vkm::mat4(1.0f), time * vkm::radians(90.0f), vkm::vec3(0, 0, 1));
    ubo.view = vkm::lookAt(vkm::vec3(2.0f, 2.0f, 2.0f), vkm::vec3(0.0f, 0.0f, 0.0f), vkm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj =
        vkm::perspective(vkm::radians(45.0f), swapchain_extend_.width / (float)swapchain_extend_.height, 0.1f, 10.0f);

    memcpy(uniform_buffers_map_[current_image], &ubo, sizeof(ubo));
}

void Acvv::draw_frame()
{
    if (vkWaitForFences(device_, 1, &frame_fence_[current_frame_], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    {
        throw std::runtime_error("in flight fence error\n");
    }

    uint32_t image_index = 0;
    vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, get_image_semaphores_[current_frame_], VK_NULL_HANDLE,
                          &image_index);
    vkResetFences(device_, 1, &frame_fence_[current_frame_]);

    update_ubo(current_frame_);
    vkResetCommandBuffer(command_buffers_[current_frame_], 0);
    record_command(command_buffers_[current_frame_], image_index);

    VkSemaphore wait_semaphores[] = {get_image_semaphores_[current_frame_]};
    VkSemaphore signal_semaphores[] = {image_render_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers_[current_frame_];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    vkQueueSubmit(graphics_queue_, 1, &submit_info, frame_fence_[current_frame_]);

    VkSwapchainKHR swapchains[] = {swapchain_};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(present_queue_, &present_info);
    current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}
