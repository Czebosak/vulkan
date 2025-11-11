#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>
#include <optional>

namespace hayvk::builders {
    class PipelineBuilder {
    public:
        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
    
        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        vk::PipelineRasterizationStateCreateInfo rasterizer;
        vk::PipelineColorBlendAttachmentState color_blend_attachment;
        vk::PipelineMultisampleStateCreateInfo multisampling;
        vk::PipelineLayoutCreateInfo pipeline_layout_create_info;
        std::optional<vk::PipelineDepthStencilStateCreateInfo> depth_stencil;
        vk::Format color_attachment_format;

        std::optional<vk::Pipeline> build(vk::Device device);

        PipelineBuilder& set_shaders(vk::ShaderModule vertex_shader, vk::ShaderModule fragment_shader, const char* entry = "main");
        PipelineBuilder& set_multisampling_none();
    };
}
