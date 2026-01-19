#include "voxel_renderer.hpp"
#include "backends/vulkan/allocated_buffer.hpp"
#include "voxel/render_types.hpp"

#include <fmt/core.h>

#include <iostream>

#include <backends/vulkan/defines.hpp>
#include <backends/vulkan/initializers.hpp>
#include <backends/vulkan/pipeline_builder.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <voxel/mesh_generation.hpp>

namespace voxel::renderer {
    VoxelRenderer::Buffer VoxelRenderer::Buffer::create(RenderState& render_state, bool cpu_only) {
        VoxelRenderer::Buffer buffer;
        VkBufferUsageFlags usage;
        VmaMemoryUsage memory_usage;
        if (!cpu_only) {
            usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        } else {
            usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
        }
        buffer.gpu_buffer = AllocatedBuffer::create(render_state, TOTAL_BUFFER_SIZE, usage, memory_usage);

        if (!cpu_only) {
            VkBufferDeviceAddressInfo device_adress_info = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer.gpu_buffer.buffer };
            buffer.addr = vkGetBufferDeviceAddress(render_state.device, &device_adress_info);
        }

        buffer.blocks.set();

        //fmt::println("new buffer alllocated");
        return buffer;
    }

    VoxelRenderer::Buffer::ContinuityResult VoxelRenderer::Buffer::check_continuity(size_t i, size_t needed_length) {
        size_t contig_i = i;
        while (contig_i != blocks.size() && blocks[contig_i] != 0) {
            if (contig_i - i + 1 == needed_length) {
                for (size_t set_i = i; set_i <= contig_i; set_i++) {
                    blocks[set_i] = 0;
                }
                return { true };
            }
            contig_i++;
        }
        return { false, contig_i };
    }

    inline size_t VoxelRenderer::Buffer::calculate_needed_blocks(size_t bytes) {
        return (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }

    size_t VoxelRenderer::Buffer::allocate(size_t n) {
        size_t i = 0;

        auto res = check_continuity(i, n);
        if (res.found) return i;
        i = res.new_i;

        while ((i = blocks._Find_next(i)) != blocks.size()) {
            size_t contig_i = i;
            auto res = check_continuity(i, n);
            if (res.found) return i;

            i = res.new_i;
        }

        return BLOCK_COUNT;
    }

    // In blocks
    void VoxelRenderer::Buffer::free(size_t i, size_t length) {
        for (size_t bit_i = i; bit_i < (i + length); bit_i++) {
            blocks[bit_i] = 1;
        }
    }

    void VoxelRenderer::Buffer::destroy(RenderState& render_state) {
        gpu_buffer.destroy(render_state.allocator);
    }

    inline VkBuffer VoxelRenderer::Buffer::get_buffer_handle() {
        return gpu_buffer.buffer;
    }

    inline AllocatedBuffer& VoxelRenderer::Buffer::get_allocated_buffer() {
        return gpu_buffer;
    }

    inline VkDeviceAddress VoxelRenderer::Buffer::get_buffer_addr() {
        return addr;
    }

    void VoxelRenderer::Buffer::print() const {
        for (int i = 0; i < 64; i++)
            fmt::print("{}", (int)!(blocks[i]));
        fmt::print("\n");
    }

    void VoxelRenderer::init(RenderState& render_state) {
        VkCommandPoolCreateInfo command_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = render_state.queue_family_index,
        };

        VK_CHECK(vkCreateCommandPool(render_state.device, &command_pool_info, nullptr, &command_pool));

        VkCommandBufferAllocateInfo cmd_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1,
        };

        VK_CHECK(vkAllocateCommandBuffers(render_state.device, &cmd_alloc_info, &render_cmd_buffer));

        VkShaderModule voxel_frag_shader;
        auto voxel_frag_shader_opt = vkutil::load_shader_module(render_state.device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/voxel.frag.spv");
        if (voxel_frag_shader_opt) {
            voxel_frag_shader = *voxel_frag_shader_opt;
            fmt::println("Voxel fragment shader succesfully loaded");
        } else {
            fmt::println("Error when building the voxel fragment shader module");
        }

        VkShaderModule voxel_vertex_shader;
        auto voxel_vertex_shader_opt = vkutil::load_shader_module(render_state.device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/voxel.vert.spv");
        if (voxel_vertex_shader_opt) {
            voxel_vertex_shader = *voxel_vertex_shader_opt;
            fmt::println("Voxel vertex shader succesfully loaded");
        }
        else {
            fmt::println("Error when building the voxel vertex shader module");
        }
        
        VkPushConstantRange bufferRange = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(voxel::VoxelPushConstants),
        };

        VkPipelineLayoutCreateInfo pipeline_layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &bufferRange,
        };

        VK_CHECK(vkCreatePipelineLayout(render_state.device, &pipeline_layout_info, nullptr, &voxel_pipeline_layout));
        
        voxel_pipeline = hayvk::builders::PipelineBuilder { .pipeline_layout = voxel_pipeline_layout }
            .set_shaders(voxel_vertex_shader, voxel_frag_shader)
            .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
            .set_polygon_mode(VK_POLYGON_MODE_FILL)
            .set_cull_mode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .set_multisampling_none()
            .disable_blending()
            .enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL)
            .set_color_attachment_format(render_state.draw_image_format)
            .set_depth_format(render_state.depth_image_format)
            .build(render_state.device);

        vkDestroyShaderModule(render_state.device, voxel_frag_shader, nullptr);
        vkDestroyShaderModule(render_state.device, voxel_vertex_shader, nullptr);

        staging_buffer = Buffer::create(render_state, true);

        buffers.emplace_back(Buffer::create(render_state));
    }

    void VoxelRenderer::allocate_mesh(RenderState& render_state, std::span<PackedFace> data, Mesh& mesh) {
        size_t size = data.size_bytes();
        size_t needed_blocks = Buffer::calculate_needed_blocks(size);

        size_t block_i = BLOCK_COUNT;
        int buf_i = 0;
        for (; buf_i < buffers.size(); buf_i++) {
            block_i = buffers[buf_i].allocate(needed_blocks);
            if (block_i != BLOCK_COUNT) {
                break;
            }
        }

        if (block_i == BLOCK_COUNT) {
            buf_i = buffers.size();
            block_i = buffers.emplace_back(Buffer::create(render_state)).allocate(needed_blocks);
        }

        size_t staging_block_i = staging_buffer.allocate(needed_blocks);

        memcpy((uint8_t*)staging_buffer.get_allocated_buffer().info.pMappedData + (staging_block_i * BLOCK_SIZE), data.data(), size);

        mesh = Mesh {
            .allocated_addr = buffers[buf_i].get_buffer_addr() + block_i * BLOCK_SIZE,
            .buffer_index = static_cast<uint32_t>(buf_i),
            .face_count = static_cast<uint32_t>(data.size()),
            .state = MeshState::Pending,
        };

        render_state.resource_loader->add_job(resource::Job {
            .func = [block_i, staging_block_i, size, buf_i, staging_buffer = staging_buffer.get_buffer_handle(), buffers = &this->buffers, mesh_state_ptr = &mesh.state](VkCommandBuffer cmd) {
                if (*mesh_state_ptr != MeshState::Pending) {
                    return false;
                }

                VkBufferCopy data_copy = {
                    .srcOffset = staging_block_i * BLOCK_SIZE,
                    .dstOffset = block_i * BLOCK_SIZE,
                    .size = size,
                };

                vkCmdCopyBuffer(cmd, staging_buffer, (*buffers)[buf_i].get_buffer_handle(), 1, &data_copy);
                
                return true;
            },
            .callback = [mesh_state_ptr = &mesh.state, staging_buffer_ptr = &staging_buffer, staging_block_i, needed_blocks]() {
                staging_buffer_ptr->free(staging_block_i, needed_blocks);

                if (*mesh_state_ptr == MeshState::Pending) {
                    *mesh_state_ptr = MeshState::Ready;
                }
            }
        });
    }

    void VoxelRenderer::free_mesh(RenderState& render_state, const Mesh& mesh) {
        Buffer& buf = buffers[mesh.buffer_index];

        size_t block_idx = (mesh.allocated_addr - buf.get_buffer_addr()) / BLOCK_SIZE;

        buf.free(block_idx, mesh.face_count * sizeof(PackedFace));
    }

    VkCommandBuffer VoxelRenderer::draw(RenderState& render_state, ChunkManager& chunk_manager, VkFormat color_attachment_format, VkFormat depth_attachment_format, VkRenderingAttachmentInfo color_attachment, VkRenderingAttachmentInfo depth_attachment, const glm::mat4& camera_matrix) {
        VK_CHECK(vkResetCommandBuffer(render_cmd_buffer, 0));

        VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO,
            .pNext = NULL,
            .flags = 0, // usually 0
            .viewMask = 0, // for multiview; 0 if not using it
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format, // array of VkFormat
            .depthAttachmentFormat = depth_attachment_format, // VK_FORMAT_UNDEFINED if none
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED, // VK_FORMAT_UNDEFINED if none
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT // or whatever you're using
        };

        VkCommandBufferInheritanceInfo inheritance_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .pNext = &inheritance_rendering_info,
            .renderPass = VK_NULL_HANDLE, // ignored with dynamic rendering
            .subpass = 0, // ignored
            .framebuffer = VK_NULL_HANDLE, // ignored
            .occlusionQueryEnable = VK_FALSE,
            .queryFlags = 0,
            .pipelineStatistics = 0
        };

        VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &inheritance_info);
        VK_CHECK(vkBeginCommandBuffer(render_cmd_buffer, &cmd_begin_info));

        VkRenderingInfo render_info = vkinit::rendering_info(render_state.draw_extent, &color_attachment, &depth_attachment);
        vkCmdBeginRendering(render_cmd_buffer, &render_info);

        vkCmdBindPipeline(render_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, voxel_pipeline);

        VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = (float)render_state.draw_extent.width,
            .height = (float)render_state.draw_extent.height,
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };

        vkCmdSetViewport(render_cmd_buffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = render_state.draw_extent,
        };

        vkCmdSetScissor(render_cmd_buffer, 0, 1, &scissor);

        VoxelPushConstants push_constants;

        const auto& registry = registry::Registry::get();
        for (auto& [position, chunk] : chunk_manager.chunks) {
            if (chunk.mesh.state == MeshState::MarkedForCleanup) {
                free_mesh(render_state, chunk.mesh);

                chunk.mesh.state = MeshState::Dirty;
            }

            if (chunk.is_dirty()) {
                auto get_neighboring = [&](int x, int y, int z) {
                    auto it = chunk_manager.chunks.find(glm::ivec3(x, y, z));
                    return it != chunk_manager.chunks.end() ? &it->second : nullptr;
                };

                NeighboringChunks neighboring = {
                    .top    = get_neighboring(position.x, position.y + 1, position.z),
                    .bottom = get_neighboring(position.x, position.y - 1, position.z),
                    .left   = get_neighboring(position.x - 1, position.y, position.z),
                    .right  = get_neighboring(position.x + 1, position.y, position.z),
                    .front  = get_neighboring(position.x, position.y, position.z - 1),
                    .back   = get_neighboring(position.x, position.y, position.z + 1),
                };

                std::vector<PackedFace> faces = generate_mesh(render_state, chunk, registry, neighboring);

                if (faces.size() != 0) {
                    allocate_mesh(render_state, faces, chunk.mesh);
                } else {
                    chunk.mesh.state = MeshState::Ready;
                }
            }

            if (!chunk.is_mesh_ready()) {
                continue;
            }

            if (chunk.mesh.face_count == 0) {
                continue;
            }

            push_constants.mvp = camera_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(position * 16));
            push_constants.face_buffer = chunk.mesh.allocated_addr;
            vkCmdPushConstants(render_cmd_buffer, voxel_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VoxelPushConstants), &push_constants);

            vkCmdDraw(render_cmd_buffer, 4, chunk.mesh.face_count, 0, 0);
        }

        vkCmdEndRendering(render_cmd_buffer);

        VK_CHECK(vkEndCommandBuffer(render_cmd_buffer));

        return render_cmd_buffer;
    }

    void VoxelRenderer::destroy(RenderState& render_state) {
        for (Buffer& buf : buffers) {
            buf.destroy(render_state);
        }

        staging_buffer.destroy(render_state);

        vkDestroyPipelineLayout(render_state.device, voxel_pipeline_layout, nullptr);
        vkDestroyPipeline(render_state.device, voxel_pipeline, nullptr);

        vkDestroyCommandPool(render_state.device, command_pool, nullptr);
    }
}
