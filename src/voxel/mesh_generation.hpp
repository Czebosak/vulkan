#pragma once

#include <voxel/types.hpp>

#include <registry.hpp>

#include <backends/vulkan/voxel_renderer.hpp>

namespace voxel {
    struct NeighboringChunks {
        Chunk* top;
        Chunk* bottom;
        Chunk* left;
        Chunk* right;
        Chunk* front;
        Chunk* back;
    };

    std::vector<voxel::PackedFace> generate_mesh(RenderState& render_state, const voxel::Chunk& chunk, const registry::Registry& registry, NeighboringChunks neighboring_chunks);
}
