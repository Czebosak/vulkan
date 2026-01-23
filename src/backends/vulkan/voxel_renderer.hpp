#pragma once
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <bitset>

#include <backends/vulkan/render_state.hpp>
#include <backends/vulkan/allocated_buffer.hpp>
#include <backends/vulkan/allocated_image.hpp>
#include <backends/vulkan/descriptors.hpp>

#include <voxel/chunk_manager.hpp>
#include <voxel/render_types.hpp>
#include <vulkan/vulkan_core.h>

namespace voxel::renderer {
    struct VoxelPushConstants {
        glm::mat4 mvp;
        VkDeviceAddress face_buffer;
    };

    /* struct AllocInfo {
        VkDeviceAddress allocated_addr;
        size_t buffer_index;
    }; */

    class VoxelRenderer {
    private:
        constexpr static size_t BLOCK_SIZE = 4 * 1024; // 4KB
        constexpr static size_t TOTAL_BUFFER_SIZE = 64 * 1024 * 1024; // 64MB
        constexpr static size_t BLOCK_COUNT = TOTAL_BUFFER_SIZE / BLOCK_SIZE;

        static_assert(TOTAL_BUFFER_SIZE == BLOCK_SIZE * BLOCK_COUNT);

        class Buffer {
        private:
            std::bitset<BLOCK_SIZE> blocks;

            struct ContinuityResult {
                bool found;
                size_t new_i;
            };

            ContinuityResult check_continuity(size_t i, size_t needed_length);

            AllocatedBuffer gpu_buffer;
            VkDeviceAddress addr;
        public:
            static Buffer create(RenderState& render_state, bool cpu_only = false);

            static inline size_t calculate_needed_blocks(size_t bytes);

            // if allocation fails return BLOCK_COUNT
            size_t allocate(size_t n);

            void free(size_t i, size_t length);

            void destroy(RenderState& render_state);

            inline VkBuffer get_buffer_handle();

            inline AllocatedBuffer& get_allocated_buffer();

            inline VkDeviceAddress get_buffer_addr();

            void print() const;
        };
        
        VkBuffer chunk_buffer;

        VkCommandPool command_pool;
        VkCommandBuffer render_cmd_buffer;

        VkPipeline voxel_pipeline;
        VkPipelineLayout voxel_pipeline_layout;

        VkPipeline debug_pipeline;
        VkPipelineLayout debug_pipeline_layout;

        DescriptorAllocatorGrowable descriptor_allocator;

        AllocatedImage error_image;

        VkDescriptorSetLayout texture_descriptor_layout;
        VkSampler texture_sampler;

        Buffer staging_buffer;

        std::vector<Buffer> buffers;

        void init_pipeline(RenderState& render_state);
        void init_debug_pipeline(RenderState& render_state);
    public:
        void init(RenderState& render_state);

        // Size required in bytes, updates the mesh state
        void allocate_mesh(RenderState& render_state, std::span<PackedFace> data, Mesh& mesh);

        void free_mesh(RenderState& render_state, const Mesh& mesh);

        VkCommandBuffer draw(RenderState& render_state, ChunkManager& chunk_manager, VkFormat color_attachment_format, VkFormat depth_attachment_format, VkRenderingAttachmentInfo color_attachment, VkRenderingAttachmentInfo depth_attachment, const glm::mat4& camera_matrix);

        void destroy(RenderState& render_state);
    };
}
