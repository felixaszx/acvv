#include "app.hpp"

void App::setup_swap_chain()
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;

    capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);
    formats = physical_device_.getSurfaceFormatsKHR();
    present_modes = physical_device_.getSurfacePresentModesKHR();
}