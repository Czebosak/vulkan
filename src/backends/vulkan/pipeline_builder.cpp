#include "pipeline_builder.hpp"

#include <backends/vulkan/types.hpp>

namespace hayvk::builders {
    VkPipeline PipelineBuilder::build(VkDevice device) {
        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineColorBlendStateCreateInfo color_blending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamic_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = &state[0],
        };

        VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &render_info,
            .stageCount = (uint32_t)shader_stages.size(),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blending,
            .pDynamicState = &dynamic_info,
            .layout = pipeline_layout,
        };

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
            fmt::println("failed to create pipeline");
            return VK_NULL_HANDLE;
        }

        return pipeline;
    }

    PipelineBuilder& PipelineBuilder::set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader, const char* entry) {
        shader_stages.clear();

        // Vertex Shader
        shader_stages.emplace_back(VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_shader,
            .pName = entry,
        });

        // Fragment Shader
        shader_stages.emplace_back(VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_shader,
            .pName = entry,
        });

        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
        input_assembly.topology = topology;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
        rasterizer.polygonMode = mode;
        rasterizer.lineWidth = 1.f;
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode, VkFrontFace front_face) {
        rasterizer.cullMode = cull_mode;
        rasterizer.frontFace = front_face;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_multisampling_none() {
        multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };

        return *this;
    }

    PipelineBuilder& PipelineBuilder::disable_blending() {
        // default write mask
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // no blending
        color_blend_attachment.blendEnable = VK_FALSE;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_color_attachment_format(VkFormat format) {
        color_attachment_format = format;
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachmentFormats = &color_attachment_format;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::set_depth_format(VkFormat format) {
        render_info.depthAttachmentFormat = format;
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::disable_depthtest() {
        VkPipelineDepthStencilStateCreateInfo {
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = VK_COMPARE_OP_NEVER,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.f,
            .maxDepthBounds = 1.f,
        };

        return *this;
    }
}

/*

maybe try dsiabling blending 
^^^ research that please yo udum dum

.blendEnable = VK_TRUE,
.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
.colorBlendOp = VK_BLEND_OP_ADD,
.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
.alphaBlendOp = VK_BLEND_OP_ADD,
.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                | VK_COLOR_COMPONENT_G_BIT
                | VK_COLOR_COMPONENT_B_BIT
                | VK_COLOR_COMPONENT_A_BIT,

*/

std::optional<VkShaderModule> vkutil::load_shader_module(VkDevice device, const char* file_path) {
    // open the file. With cursor at the end
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) return std::nullopt;

    // find what the size of the file is by looking up the location of the cursor
    // because the cursor is at the end, it gives the size directly in bytes
    size_t file_size = (size_t)file.tellg();

    // spirv expects the buffer to be on uint32, so make sure to reserve a int
    // vector big enough for the entire file
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    // put file cursor at beginning
    file.seekg(0);

    // load the entire file into the buffer
    file.read((char*)buffer.data(), file_size);

    // now that the file is loaded into the buffer, we can close it
    file.close();

    // create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        // codeSize has to be in bytes, so multply the ints in the buffer by size of
        // int to know the real size of the buffer
        .codeSize = buffer.size() * sizeof(uint32_t),
        .pCode = buffer.data(),
    };

    // check that the creation goes well.
    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) return std::nullopt;
    return std::make_optional(shader_module);
}
