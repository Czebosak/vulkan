#include "types.hpp"

bool voxel::Chunk::is_dirty() const {
    return mesh.state == MeshState::Dirty;
}

bool voxel::Chunk::is_mesh_ready() const {
    return mesh.state == MeshState::Ready;
}

void voxel::Chunk::mark_as_dirty() {
    mesh.state = MeshState::Dirty;
}
