#include "acvv.hpp"

VkShaderModule Acvv::create_shader_module(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = castt(uint32_t, code.size());
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule tmp_module = VK_NULL_HANDLE;
    vkCreateShaderModule(device_, &create_info, nullptr, &tmp_module);

    return tmp_module;
}

void Acvv::create_graphics_pipeline()
{
}