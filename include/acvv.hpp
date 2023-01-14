#ifndef ACVV_HPP
#define ACVV_HPP

#include "ve_base.hpp"
#include "ve_device.hpp"
#include "ve_swapchain.hpp"
#include "ve_cmd.hpp"
#include "ve_buffer.hpp"
#include "ve_image.hpp"

#include "vkm.hpp"

inline const int WIDTH = 1200;
inline const int HEIGHT = 900;
inline const int MAX_FRAMES_IN_FLIGHT = 2;

#define castt(type, var) casts(type, var)

class Acvv
{
  private:
    VeBaseLayer base_layer_ = VeBaseLayer(1920, 1080);
    VeDeviceLayer device_layer_;

    VeSwapchainBase swapchain_;

    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_Layout_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapchain_framebuffers_{};

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_{};

    VeBufferBase vertex_buffer_;
    VeBufferBase index_buffer_;

    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_set_{};

    std::vector<VeBufferBase> uniform_buffers_{};
    std::vector<void*> uniform_buffers_map_{};

    uint32_t current_frame_ = 0;
    std::vector<VkSemaphore> get_image_semaphores_{};
    std::vector<VkSemaphore> image_render_semaphores_{};
    std::vector<VkFence> frame_fence_{};

    VeImageBase texture_image_;
    VkSampler textue_sampler_ = VK_NULL_HANDLE;

  public:
    void run();
    void init_window();
    void init_vulkan();
    void main_loop();
    void cleanup();

    VkShaderModule create_shader_module(const std::vector<char>& code);
    void create_render_pass();
    void setup_descriptor_set_layout();
    void create_graphics_pipeline();

    void create_sync_objs();
    void create_framebuffers();

    void create_command_buffer();

    void create_texture_image();
    void create_texture_imageview();
    void create_texture_sampler();

    void create_vertex_buffer();
    void create_index_buffer();
    void create_uniform_buffer();
    void create_descriptor_pool();

    void record_command(VkCommandBuffer command_buffer, uint32_t image_index);
    void update_ubo(uint32_t current_image);
    void draw_frame();

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};

inline std::vector<char> read_file(const std::string& file_name, std::ios_base::openmode mode)
{

    std::ifstream file(file_name, std::ios::ate | mode);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file\n");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription get_input_binding_description()
    {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions;

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, texCoord);

        return attribute_descriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, //
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  //
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},   //
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}   //
};
const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

struct UniformBufferObject
{
    vkm::mat4 model;
    vkm::mat4 view;
    vkm::mat4 proj;
};

#endif // ACVV_HPP
