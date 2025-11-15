#include "pipeline_builder.hpp"

#include <vulkan/vulkan.hpp>

namespace hayvk::builders {
    std::optional<VkPipeline> PipelineBuilder::build(VkDevice device) {
        VkPipelineViewportStateCreateInfo viewport_state = {
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineColorBlendStateCreateInfo color_blending = {
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};

        VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamic_info {
            .dynamicStateCount = 2,
            .pDynamicStates = &state[0],
        };

        VkPipelineLayout pipeline_layout;
        //vkResult result = ;
        if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineRenderingCreateInfo render_info = {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format,
        };

        VkGraphicsPipelineCreateInfo pipeline_info = {
            .pNext = &render_info,
            .stageCount = (uint32_t)shader_stages.size(),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &color_blending,
            //.pDepthStencilState(&depth_stencil)
            .pDynamicState = &dynamic_info,
            .layout = pipeline_layout,
        };

        VkPipeline pipeline;
        VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
        if (result != VK_SUCCESS) return VK_NULL_HANDLE;

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
