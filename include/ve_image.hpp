#ifndef VE_IMAGE_HPP
#define VE_IMAGE_HPP

#include "ve_base.hpp"
#include <stb/stb_image.h>

using VeImageBase = MultiType<VkImage, VkDeviceMemory, VmaAllocation, VkImageView>;

struct VeImagePixels
{
    int width = 0;
    int height = 0;
    int channel = 0;

    VkDeviceSize size = 0;
    stbi_uc* pixels = nullptr;
};

VeImagePixels load_pixel(const std::string file_name, uint32_t channel);
void free_pixel(VeImagePixels& pixel_data);

struct VePixelImageBase : public VeImageBase
{
    VeImagePixels pixels_data;
};

#endif // VE_IMAGE_HPP
