#include "types.hpp"

bool voxel::Chunk::is_dirty() {
    return std::holds_alternative<voxel::Dirty>(mesh_state);
}

void voxel::Chunk::mark_as_dirty() {
    mesh_state = Dirty {};
}
