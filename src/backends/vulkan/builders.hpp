#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <optional>

namespace hayvk::builders {
    std::optional<vk::UniqueShaderModule> load_shader_module(vk::Device device, const char* file_path);
}
