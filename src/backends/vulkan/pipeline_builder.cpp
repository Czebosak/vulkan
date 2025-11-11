#include "pipeline_builder.hpp"

#include <vulkan/vulkan.hpp>

namespace hayvk::builders {
    std::optional<vk::Pipeline> PipelineBuilder::build(vk::Device device) {
        vk::PipelineViewportStateCreateInfo viewport_state{};
        viewport_state.setPNext(nullptr)
                      .setViewportCount(1)
                      .setScissorCount(1);

        vk::PipelineColorBlendStateCreateInfo color_blending{};
        color_blending.setLogicOpEnable(VK_FALSE)
                      .setLogicOp(vk::LogicOp::eCopy)
                      .setAttachmentCount(1)
                      .setPAttachments(&color_blend_attachment);

        vk::PipelineVertexInputStateCreateInfo vertex_input_info{};

        vk::DynamicState state[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        vk::PipelineDynamicStateCreateInfo dynamic_info({}, 2, &state[0]);

        vk::PipelineLayout pipeline_layout;
        //vk::Result result = ;
        if (device.createPipelineLayout(&pipeline_layout_create_info, nullptr, &pipeline_layout) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        vk::PipelineRenderingCreateInfo render_info{};
        
        render_info.setColorAttachmentCount(1)
                   .setPColorAttachmentFormats(&color_attachment_format);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setPNext(&render_info)
                     .setStageCount((uint32_t)shader_stages.size())
                     .setPStages(shader_stages.data())
                     .setLayout(pipeline_layout)
                     .setPVertexInputState(&vertex_input_info)
                     .setPInputAssemblyState(&input_assembly)
                     .setPViewportState(&viewport_state)
                     .setPRasterizationState(&rasterizer)
                     .setPMultisampleState(&multisampling)
                     .setPDepthStencilState(nullptr)
                     .setPColorBlendState(&color_blending)
                     //.pDepthStencilState(&depth_stencil)
                     .setPDynamicState(&dynamic_info);

        vk::Pipeline pipeline;
        vk::Result result = device.createGraphicsPipelines({}, 1, &pipeline_info, nullptr, &pipeline);
        if (result != vk::Result::eSuccess) return VK_NULL_HANDLE;

        return pipeline;
    }

    PipelineBuilder& PipelineBuilder::set_shaders(vk::ShaderModule vertex_shader, vk::ShaderModule fragment_shader, const char* entry) {
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