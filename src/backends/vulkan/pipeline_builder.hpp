#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <fstream>

namespace hayvk::builders {
    class PipelineBuilder {
    public:
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    
        VkPipelineInputAssemblyStateCreateInfo input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        VkPipelineRasterizationStateCreateInfo rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipeline_layout;
        VkPipelineDepthStencilStateCreateInfo depth_stencil;
        VkPipelineRenderingCreateInfo render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        VkFormat color_attachment_format;

        VkPipeline build(VkDevice device);

        PipelineBuilder& set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader, const char* entry = "main");
        PipelineBuilder& set_input_topology(VkPrimitiveTopology topology);
        PipelineBuilder& set_polygon_mode(VkPolygonMode mode);
        PipelineBuilder& set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face);
        PipelineBuilder& set_multisampling_none();
        PipelineBuilder& disable_blending();
        PipelineBuilder& set_color_attachment_format(VkFormat format);
        PipelineBuilder& set_depth_format(VkFormat format);
        PipelineBuilder& disable_depthtest();
    };
}

namespace vkutil {
    std::optional<VkShaderModule> load_shader_module(VkDevice device, const char* file_path);
}
