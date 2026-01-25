#include "mesh_generation.hpp"

#include <vector>
#include <ranges>

#include <voxel/render_types.hpp>

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_utility.hpp>

using namespace voxel;

consteval uint8_t face_to_bit(Face f) {
    return 1 << magic_enum::enum_index(f).value();
}

/* uint8_t generate_visible_faces(std::vector<PackedFace>& packed_faces, size_t x, size_t y, size_t z, const voxel::Chunk& chunk, BlockID air_id) {
    uint8_t faces = 0;

    if (z == 0              || chunk.data[x][y][z - 1].id == air_id) faces |= face_to_bit(Face::Forward);
    if (z == CHUNK_SIZE - 1 || chunk.data[x][y][z + 1].id == air_id) faces |= face_to_bit(Face::Backward);
    if (x == 0              || chunk.data[x - 1][y][z].id == air_id) faces |= face_to_bit(Face::Left);
    if (x == CHUNK_SIZE - 1 || chunk.data[x + 1][y][z].id == air_id) faces |= face_to_bit(Face::Right);
    if (y == 0              || chunk.data[x][y - 1][z].id == air_id) faces |= face_to_bit(Face::Down);
    if (y == CHUNK_SIZE - 1 || chunk.data[x][y + 1][z].id == air_id) faces |= face_to_bit(Face::Up);

    return faces;
} */

inline uint32_t pack_face(size_t x, size_t y, size_t z, Face face) {
    static_assert(magic_enum::enum_integer(magic_enum::enum_values<Face>().back()) <= (1 << 3));

    uint32_t packed = (x << 0) | (y << 5) | (z << 10) | (magic_enum::enum_integer(face) << 15);

    return packed;
}

inline void generate_visible_faces(std::vector<PackedFace>& packed_faces, size_t x, size_t y, size_t z, const voxel::Chunk& chunk, NeighboringChunks& n, BlockID air_id) {
    auto create_face = [&](Face f) {
        packed_faces.emplace_back(pack_face(x, y, z, f));
    };

    // Back
    if (z == 0) {
        if (n.back && n.back->data[x][y][CHUNK_SIZE - 1].id == air_id) create_face(Face::Back);
    } else if (chunk.data[x][y][z - 1].id == air_id) {
        create_face(Face::Back);
    }

    // Front
    if (z == CHUNK_SIZE - 1) {
         if (n.front && n.front->data[x][y][0].id == air_id) create_face(Face::Front);
    } else if (chunk.data[x][y][z + 1].id == air_id) {
        create_face(Face::Front);
    }

    // Left
    if (x == 0) {
        if (n.left && n.left->data[CHUNK_SIZE - 1][y][z].id == air_id) create_face(Face::Left);
    } else if (chunk.data[x - 1][y][z].id == air_id) {
        create_face(Face::Left);
    }

    // Right
    if (x == CHUNK_SIZE - 1) {
         if (n.right && n.right->data[0][y][z].id == air_id) create_face(Face::Right);
    } else if (chunk.data[x + 1][y][z].id == air_id) {
        create_face(Face::Right);
    }

    // Bottom
    if (y == 0) {
         if (n.bottom && n.bottom->data[x][CHUNK_SIZE - 1][z].id == air_id) create_face(Face::Bottom);
    } else if (chunk.data[x][y - 1][z].id == air_id) {
        create_face(Face::Bottom);
    }

    // Top
    if (y == CHUNK_SIZE - 1) {
         if (n.top && n.top->data[x][0][z].id == air_id) create_face(Face::Top);
    } else if (chunk.data[x][y + 1][z].id == air_id) {
        create_face(Face::Top);
    }
}

inline uint32_t pack_block_data(BlockID id) {
    return (id & 0x7FFF) << 17;
}

std::pair<std::vector<PackedFace>, std::vector<uint32_t>> voxel::generate_mesh(RenderState& render_state, const voxel::Chunk& chunk, const registry::Registry& registry, NeighboringChunks neighboring_chunks) {
    // Face instance data:
    // 00000000000000FFFYYYYYZZZZZXXXXX
    // F - face direction
    // X, Y, Z position

    // Block data
    // BBBBBBBBBBBBBBBAAAAARRRRGGGGBBBB
    // B - block id
    // A - ambient occlusion data
    // RGB - lighting channels

    std::vector<PackedFace> packed_faces;
    std::vector<uint32_t> block_data;

    auto iota = std::views::iota(size_t(0), CHUNK_SIZE);

    BlockID air_id = registry.get_block("skibidi:air")->id;

    for (auto [x, y, z] : std::views::cartesian_product(iota, iota, iota)) {
        block_data.emplace_back(pack_block_data(chunk.data[x][y][z].id));

        if (chunk.data[x][y][z].id == air_id) {
            continue;
        }

        generate_visible_faces(packed_faces, x, y, z, chunk, neighboring_chunks, air_id);
    }

    return std::make_pair(std::move(packed_faces), std::move(block_data));
};
