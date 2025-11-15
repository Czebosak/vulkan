#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <fstream>

namespace hayvk::builders {
    class PipelineBuilder {
    public:
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    
        VkPipelineInputAssemblyStateCreateInfo input_assembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayoutCreateInfo pipeline_layout_create_info;
        std::optional<VkPipelineDepthStencilStateCreateInfo> depth_stencil;
        VkFormat color_attachment_format;

        std::optional<VkPipeline> build(VkDevice device);

        PipelineBuilder& set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader, const char* entry = "main");
        PipelineBuilder& set_multisampling_none();
    };
}

namespace vkutil {
    std::optional<VkShaderModule> load_shader_module(VkDevice device, const char* file_path);
}
