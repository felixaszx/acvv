#include "ve_base.hpp"

VeBaseLayer::VeBaseLayer(uint32_t width, uint32_t height, bool debug)
    : WIDTH(width),
      HEIGHT(height),
      ENABLE_VALIDATION_LAYERS(debug)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

VeBaseLayer::~VeBaseLayer()
{
    glfwTerminate();
}

void VeBaseLayer::create(const std::string& name)
{
    *this = glfwCreateWindow(WIDTH, HEIGHT, name.c_str(), nullptr, nullptr);

    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support())
    {
        throw std::runtime_error("Validation layer is not supported\n");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "ve";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_creat_info{};
    debug_utils_creat_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_creat_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | //
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_utils_creat_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |    //
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | //
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_creat_info.pfnUserCallback = debug_cb;

    auto exts = get_required_exts();
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.enabledExtensionCount = casts(uint32_t, exts.size());
    instance_create_info.ppEnabledExtensionNames = exts.data();

    if (ENABLE_VALIDATION_LAYERS)
    {
        instance_create_info.enabledLayerCount = casts(uint32_t, VALIDATION_LAYERS.size());
        instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        instance_create_info.pNext = casts(VkDebugUtilsMessengerCreateInfoEXT*, &debug_utils_creat_info);
    }
    else
    {
        instance_create_info.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&instance_create_info, nullptr, this->ptr()) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create vulkan instance\n");
    }

    if (ENABLE_VALIDATION_LAYERS)
    {
        auto load_func = load_ext_function<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr, this->data<VkInstance>(), "vkCreateDebugUtilsMessengerEXT");
        if (load_func(*this, &debug_utils_creat_info, nullptr, this->ptr()) != VK_SUCCESS)
        {

            throw std::runtime_error("Do not create validation layers\n");
        }
    }

    if (glfwCreateWindowSurface(*this, *this, nullptr, this->ptr()) != VK_SUCCESS)
    {
        throw std::runtime_error("Do not create window surface\n");
    }
}

void VeBaseLayer::destroy()
{
    if (ENABLE_VALIDATION_LAYERS)
    {
        auto load_func = load_ext_function<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr, this->data<VkInstance>(), "vkDestroyDebugUtilsMessengerEXT");
        load_func(*this, *this, nullptr);
    }
    vkDestroySurfaceKHR(*this, *this, nullptr);
    vkDestroyInstance(*this, nullptr);
    glfwDestroyWindow(*this);
}

//
// helpers
//

bool VeBaseLayer::check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layer_present(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layer_present.data());

    for (const char* layer_name : VALIDATION_LAYERS)
    {
        bool layer_found = false;
        for (const auto& layer_properties : layer_present)
        {
            if (!strcmp(layer_name, layer_properties.layerName))
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
        {
            return false;
        }
    }

    return true;
}

VkBool32 VKAPI_CALL VeBaseLayer::debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    auto type = [messageSeverity]()
    {
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            {
                return "WARNING";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            {
                return "ERROR";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            {
                return "INFO";
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            {
                return "VERBOSE";
            }
            default:
            {
                return "UNDEFINE";
            }
        }
    };
    std::cerr << fmt::format("[Vulkan Validation Layer: {}] {}\n\n", type(), pCallbackData->pMessage);

    return VK_FALSE;
}

std::vector<const char*> VeBaseLayer::get_required_exts()
{
    uint32_t glfwExtCount = 0;
    const char** glfwExts = nullptr;
    glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char*> exts(glfwExts, glfwExts + glfwExtCount);

    if (ENABLE_VALIDATION_LAYERS)
    {
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return exts;
}

std::vector<char> read_file(const std::string& file_name, std::ios_base::openmode mode)
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

VeCpuTimer::VeCpuTimer()
{
    init = std::chrono::high_resolution_clock::now();
}

float VeCpuTimer::since_init_second()
{
    return std::chrono::duration<float, std::chrono::seconds::period> //
        (std::chrono::high_resolution_clock::now() - init).count();
}

uint32_t VeCpuTimer::since_init_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds> //
        (std::chrono::high_resolution_clock::now() - init).count();
}

void VeCpuTimer::start()
{
    begin = std::chrono::high_resolution_clock::now();
}

void VeCpuTimer::finish()
{
    end = std::chrono::high_resolution_clock::now();
}

float VeCpuTimer::get_duration_second()
{
    return std::chrono::duration<float, std::chrono::seconds::period>(begin - end).count();
}

uint32_t VeCpuTimer::get_duration_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

void VeMouseTracker::init(GLFWwindow* window)
{
    glfwGetCursorPos(window, &last_x, &last_y);
}

void VeMouseTracker::update(GLFWwindow* window)
{
    last_x = this_x;
    last_y = this_y;
    glfwGetCursorPos(window, &this_x, &this_y);
}

VeMouseTracker::VeMousePosition2D VeMouseTracker::get_last_position()
{
    return {casts(float, last_x), casts(float, last_y)};
}

VeMouseTracker::VeMousePosition2D VeMouseTracker::get_this_position()
{
    return {casts(float, this_x), casts(float, this_y)};
}

VeMouseTracker::VeMousePosition2D VeMouseTracker::get_delta_position()
{
    return {casts(float, this_x - last_x), casts(float, this_y - last_y)};
}
