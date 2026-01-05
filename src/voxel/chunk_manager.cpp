#include "chunk_manager.hpp"

#include <voxel/mesh_generation.hpp>

inline int square(int n) {
    return n * n;
}

void voxel::ChunkManager::update(glm::dvec3 player_position, int hrender_distance, int vrender_distance, std::function<VkDeviceAddress(size_t)> allocation_function) {
    glm::ivec3 player_chunk_pos = player_position / 16.0;

    int a = square(hrender_distance);
    int c = square(vrender_distance);
    int ac = a * c;

    for (int chunk_x = -hrender_distance; chunk_x < hrender_distance; chunk_x++) {
        for (int chunk_y = -vrender_distance; chunk_y < vrender_distance; chunk_y++) {
            for (int chunk_z = -hrender_distance; chunk_z < hrender_distance; chunk_z++) {
                if (square(chunk_x) * c + square(chunk_y) * a + (square(chunk_z) * c) <= ac) {
                    glm::ivec3 chunk_pos = glm::ivec3(chunk_x, chunk_y, chunk_z) + player_chunk_pos;
                    if (!chunks.contains(chunk_pos)) {
                        chunks_to_load.push(chunk_pos);
                    }
                }
            }
        }
    }
    
    const auto& registry = registry::Registry::get();

    while (!chunks_to_load.empty()) {
        glm::ivec3 chunk_pos = chunks_to_load.back();
        chunks_to_load.pop();

        chunks.emplace(std::make_pair(chunk_pos, world_generator.generate_chunk(chunk_pos, registry)));
    }
}
