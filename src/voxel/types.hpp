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

    enum class Face : uint8_t {
        Top,
        Bottom,
        Left,
        Right,
        Front,
        Back
    };

    /* struct MeshFace {
        VkBuffer buffer;
        VkDeviceAddress buffer_addr;
        uint32_t instance_count;
    };

    struct Mesh {
        MeshFace faces[6];
    }; */

    enum class MeshState {
        Ready,
        Pending,
        Dirty,
        MarkedForCleanup,
    };

    struct Mesh {
        VkDeviceAddress allocated_addr;
        uint32_t buffer_index;
        uint32_t face_count;
        MeshState state;
    };

    struct Chunk {
        std::array<std::array<std::array<Block, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> data;
        Mesh mesh;

        bool is_dirty() const;
        bool is_mesh_ready() const;
        void mark_as_dirty();
    };
}
