#include "mesh_generation.hpp"

#include <vector>
#include <algorithm>
#include <ranges>

#include <voxel/render_types.hpp>

using namespace voxel;

voxel::Mesh generate_mesh(RenderState& render_state, const voxel::Chunk& chunk, const registry::Registry& registry) {
    std::vector<voxel::Vertex> vertices;
    std::vector<uint16_t> indices;

    auto iota = std::views::iota(size_t(0), CHUNK_SIZE);

    BlockID air_id = registry.get_block("skibidi:air")->id;

    int i = 0;
    for (auto [x, y, z] : std::views::cartesian_product(iota, iota, iota)) {
        if (chunk.data[x][y][z].id == air_id) {
            continue;
        }

        glm::vec3 voxel_pos = {x, y, z};
        std::array<voxel::Vertex, 8> cube_vertices = {
            voxel::Vertex { glm::vec3(0.0f, 0.0f, 1.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(1.0f, 0.0f, 1.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(0.0f, 1.0f, 1.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(1.0f, 1.0f, 1.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(0.0f, 0.0f, 0.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(1.0f, 0.0f, 0.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(0.0f, 1.0f, 0.0f) + voxel_pos },
            voxel::Vertex { glm::vec3(1.0f, 1.0f, 0.0f) + voxel_pos },
        };

        vertices.insert(vertices.end(), cube_vertices.begin(), cube_vertices.end());

        /* std::array<uint16_t, 36> cube_indices = {
            static_cast<uint16_t>(0+i), static_cast<uint16_t>(1+i), static_cast<uint16_t>(3+i), static_cast<uint16_t>(3+i), static_cast<uint16_t>(1+i), static_cast<uint16_t>(2+i),
            static_cast<uint16_t>(1+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(6+i),
            static_cast<uint16_t>(5+i), static_cast<uint16_t>(4+i), static_cast<uint16_t>(6+i), static_cast<uint16_t>(6+i), static_cast<uint16_t>(4+i), static_cast<uint16_t>(7+i),
            static_cast<uint16_t>(4+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(3+i),
            static_cast<uint16_t>(3+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(6+i),
            static_cast<uint16_t>(4+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(1+i),
        }; */
        std::array<uint16_t, 36> cube_indices = {
            // front (z = 1)
            0+i, 1+i, 2+i, 2+i, 1+i, 3+i,
            // right (x = 1)
            1+i, 5+i, 3+i, 3+i, 5+i, 7+i,
            // back (z = 0)
            5+i, 4+i, 7+i, 7+i, 4+i, 6+i,
            // left (x = 0)
            4+i, 0+i, 6+i, 6+i, 0+i, 2+i,
            // top (y = 1)
            2+i, 3+i, 6+i, 6+i, 3+i, 7+i,
            // bottom (y = 0)
            4+i, 5+i, 0+i, 0+i, 5+i, 1+i,
        };

        indices.insert(indices.end(), cube_indices.begin(), cube_indices.end());

        i += 8;
    }

    GPUMeshBuffers buffers = upload_mesh(render_state, std::span(vertices), std::span(indices));

    return voxel::Mesh { buffers, uint32_t(indices.size()) };
};
