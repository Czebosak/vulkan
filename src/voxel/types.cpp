#include "types.hpp"

bool voxel::Chunk::is_dirty() {
    return std::holds_alternative<voxel::Dirty>(mesh_state);
}
