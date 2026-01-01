#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <variant>

#include <backends/vulkan/allocated_buffer.hpp>

namespace voxel {
    using BlockID = uint32_t;

    struct Block {
        BlockID id;
    };

    constexpr size_t CHUNK_SIZE = 16;

    struct Dirty {};

    struct Mesh {
        GPUMeshBuffers buffers;
        uint32_t index_count;
    };

    using MeshState = std::variant<Mesh, Dirty>;

    struct Chunk {
        std::array<std::array<std::array<Block, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> data;
        MeshState mesh_state;

        bool is_dirty();
    };
}
