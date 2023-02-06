#include "ve_image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

VeImagePixels load_pixel(const std::string file_name, uint32_t channel)
{
    VeImagePixels image_pixels{};

    image_pixels.pixels =
        stbi_load("res/texture.jpg", &image_pixels.width, &image_pixels.height, &image_pixels.channel, channel);
    if (image_pixels.pixels == nullptr)
    {
        return VeImagePixels();
    }
    image_pixels.size = image_pixels.width * image_pixels.height * channel;

    return image_pixels;
}

void free_pixel(VeImagePixels& pixel_data)
{
    stbi_image_free(pixel_data.pixels);
    pixel_data = VeImagePixels();
}
