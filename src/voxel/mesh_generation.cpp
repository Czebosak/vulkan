#include "mesh_generation.hpp"

#include <vector>
#include <algorithm>
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

uint32_t pack_face(size_t x, size_t y, size_t z, Face face) {
    static_assert(magic_enum::enum_integer(magic_enum::enum_values<Face>().back()) <= (1 << 3));

    uint32_t packed = (x << 0) | (y << 5) | (z << 10) | (magic_enum::enum_integer(face) << 15);

    return packed;
}

void generate_visible_faces(std::vector<PackedFace>& packed_faces, size_t x, size_t y, size_t z, const voxel::Chunk& chunk, BlockID air_id) {
    if (z != 0              && chunk.data[x][y][z - 1].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Back));
    if (z != CHUNK_SIZE - 1 && chunk.data[x][y][z + 1].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Front));
    if (x != 0              && chunk.data[x - 1][y][z].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Left));
    if (x != CHUNK_SIZE - 1 && chunk.data[x + 1][y][z].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Right));
    if (y != 0              && chunk.data[x][y - 1][z].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Bottom));
    if (y != CHUNK_SIZE - 1 && chunk.data[x][y + 1][z].id == air_id) packed_faces.emplace_back(pack_face(x, y, z, Face::Top));
}

std::vector<PackedFace> voxel::generate_mesh(RenderState& render_state, const voxel::Chunk& chunk, const registry::Registry& registry) {
    // Face instance data:
    // 00000000000000FFFYYYYYZZZZZXXXXX
    // F - face direction
    // X, Y, Z position

    std::vector<PackedFace> packed_faces;

    auto iota = std::views::iota(size_t(0), CHUNK_SIZE);

    BlockID air_id = registry.get_block("skibidi:air")->id;

    for (auto [x, y, z] : std::views::cartesian_product(iota, iota, iota)) {
        if (chunk.data[x][y][z].id == air_id) {
            continue;
        }

        generate_visible_faces(packed_faces, x, y, z, chunk, air_id);
    }

    return packed_faces;
};
