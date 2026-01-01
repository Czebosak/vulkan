#pragma once

#include <voxel/types.hpp>

#include <registry.hpp>

voxel::Mesh generate_mesh(RenderState& render_state, const voxel::Chunk& chunk, const registry::Registry& registry);
