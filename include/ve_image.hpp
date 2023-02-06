#ifndef VE_IMAGE_HPP
#define VE_IMAGE_HPP

#include "ve_base.hpp"
#include "ve_device.hpp"
#include "ve_buffer.hpp"
#include <stb/stb_image.h>

using VeImageBase = MultiType<VkImage, VkDeviceMemory, VmaAllocation, VkImageView>;

struct VeImagePixels
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channel = 0;

    VkDeviceSize size = 0;
    stbi_uc* pixels = nullptr;
};

VeImagePixels load_pixel(const std::string file_name, uint32_t channel);
void free_pixel(VeImagePixels& pixel_data);

struct VeTextureBase : public VeImageBase
{
  private:
    void copy_buffer_to_image(VeDeviceLayer device_layer, VkCommandPool pool);
    void transit_to_dst(VeDeviceLayer device_layer, VkCommandPool pool);
    void transit_to_shader(VeDeviceLayer device_layer, VkCommandPool pool);

  public:
    VeImagePixels pixels_data{};
    VeTextureBase(const std::string& file_name);

    void create(VeDeviceLayer device_layer, VkCommandPool pool);
    void destroy(VeDeviceLayer device_layer);
};

#endif // VE_IMAGE_HPP
