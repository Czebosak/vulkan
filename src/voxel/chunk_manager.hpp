#pragma once

#include <unordered_map>
#include <functional>
#include <queue>

#include <voxel/types.hpp>
#include <voxel/world_generator.hpp>

#include <glm/glm.hpp>

namespace voxel {
    namespace renderer {
        class VoxelRenderer;
    }

    class ChunkManager {
    private:
        WorldGenerator world_generator;

        struct IVec3Hash {
            std::size_t operator()(const glm::ivec3& v) const noexcept {
                std::size_t h1 = std::hash<int>()(v.x);
                std::size_t h2 = std::hash<int>()(v.y);
                std::size_t h3 = std::hash<int>()(v.z);

                std::size_t seed = h1;
                seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
        };

        std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;

        std::queue<glm::ivec3> chunks_to_load;

        friend renderer::VoxelRenderer;
    public:
        void update(glm::dvec3 player_position, int hrender_distance, int vrender_distance);
    };
}
