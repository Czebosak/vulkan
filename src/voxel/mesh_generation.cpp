#include "mesh_generation.hpp"

#include <vector>
#include <algorithm>
#include <ranges>

#include <voxel/render_types.hpp>

using namespace voxel;

voxel::Mesh generate_mesh(RenderState& render_state) {
    std::vector<voxel::Vertex> vertices(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 8);
    std::vector<uint16_t> indices(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 36);

    auto iota = std::views::iota(size_t(0), CHUNK_SIZE);

    int i = 0;
    for (auto [x, y, z] : std::views::cartesian_product(iota, iota, iota)) {
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

        std::array<uint16_t, 36> cube_indices = {
            static_cast<uint16_t>(0+i), static_cast<uint16_t>(1+i), static_cast<uint16_t>(3+i), static_cast<uint16_t>(3+i), static_cast<uint16_t>(1+i), static_cast<uint16_t>(2+i),
            static_cast<uint16_t>(1+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(6+i),
            static_cast<uint16_t>(5+i), static_cast<uint16_t>(4+i), static_cast<uint16_t>(6+i), static_cast<uint16_t>(6+i), static_cast<uint16_t>(4+i), static_cast<uint16_t>(7+i),
            static_cast<uint16_t>(4+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(3+i),
            static_cast<uint16_t>(3+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(7+i), static_cast<uint16_t>(2+i), static_cast<uint16_t>(6+i),
            static_cast<uint16_t>(4+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(0+i), static_cast<uint16_t>(5+i), static_cast<uint16_t>(1+i),
        };

        indices.insert(indices.end(), cube_indices.begin(), cube_indices.end());

        i += 8;
    }

    //AllocatedBuffer vertex_buffer = AllocatedBuffer::create(render_state, vertices.size() * sizeof(voxel::Vertex), );
    //AllocatedBuffer index_buffer  = AllocatedBuffer::create(render_state, indices.size()  * sizeof(uint16_t),      );
    GPUMeshBuffers buffers = upload_mesh(render_state, std::span(vertices), std::span(indices));
};
