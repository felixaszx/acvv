#include "app.hpp"

void App::setup_physical_device()
{
    auto physical_devices = instance_.enumeratePhysicalDevices();

    for (auto& device : physical_devices)
    {
        auto queue_properties = device.getQueueFamilyProperties();
        auto supported_exts = device.enumerateDeviceExtensionProperties();

        std::cout<<1;
    }
}