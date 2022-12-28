#include "app.hpp"

void App::record_command(vk::CommandBuffer command_buffer, uint32_t image_index)
{
    vk::CommandBufferBeginInfo begin_info{};
    command_buffer.begin(begin_info);

    vk::ClearValue clear_color({0.0f, 0.0f, 0.0f, 1.0f});
    vk::RenderPassBeginInfo render_pass_info{};
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = swapchain_framebuffers[image_index];
    render_pass_info.renderArea.extent = swapchain_extend;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    command_buffer.beginRenderPass(&render_pass_info, vk::SubpassContents::eInline);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
    vk::Buffer vertex_buffers[] = {vertex_buffer};
    vk::DeviceSize offsets[] = {0};
    command_buffer.bindVertexBuffers(0, vertex_buffers, offsets);

    vk::Viewport viewport{};
    viewport.width = (uint32_t)swapchain_extend.width;
    viewport.height = (uint32_t)swapchain_extend.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    command_buffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.extent = swapchain_extend;
    command_buffer.setScissor(0, scissor);

    command_buffer.draw(vertices.size(), 1, 0, 0);

    command_buffer.endRenderPass();
    command_buffer.end();
}

void App::draw_frame()
{
    if (device_.waitForFences(in_flights[current_frame], VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
    {
        throw std::runtime_error("in flight fence error\n");
    }

    auto image_detail = device_.acquireNextImageKHR(swapchain, UINT64_MAX, image_semaphores[current_frame]);
    uint32_t image_index = image_detail.value;
    device_.resetFences(in_flights[current_frame]);

    command_buffers[current_frame].reset();
    record_command(command_buffers[current_frame], image_index);

    vk::Semaphore wait_semaphores[] = {image_semaphores[current_frame]};
    vk::Semaphore signal_semaphores[] = {render_semaphores[current_frame]};
    vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submit_info{};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    graphics_queue_.submit(submit_info, in_flights[current_frame]);

    vk::SwapchainKHR swapchains[] = {swapchain};
    vk::PresentInfoKHR present_info{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    vk::Result result = present_queue_.presentKHR(present_info);
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
