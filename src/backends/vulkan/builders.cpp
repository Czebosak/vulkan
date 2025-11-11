#include "builders.hpp"

#include <fstream>
#include <iostream>

namespace hayvk::builders {
    std::optional<vk::UniqueShaderModule> load_shader_module(vk::Device device, const char* file_path) {
        std::ifstream file(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t file_size = (size_t) file.tellg();
        std::vector<uint32_t> buffer((file_size / sizeof(uint32_t)) * sizeof(char));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), file_size);

        file.close();

        vk::ShaderModuleCreateInfo create_info(
            {},
            file_size,
            reinterpret_cast<const uint32_t*>(buffer.data())
        );

        vk::ResultValue<vk::UniqueShaderModule> result = device.createShaderModuleUnique(create_info);

        if (result.result == vk::Result::eSuccess) {
            return std::make_optional(std::move(result.value));
        } else {
            std::cerr << "Failed to create shader module: " << vk::to_string(result.result) << std::endl;
            return std::nullopt;
        }
    }
}
