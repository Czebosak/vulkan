#pragma once

#include <backends/generic/camera.hpp>

#include <input.hpp>

#include <voxel/types.hpp>
#include <voxel/world_generator.hpp>
#include <voxel/chunk_manager.hpp>

class GameState {
public:
    Camera camera;

    voxel::ChunkManager chunk_manager;

    WorldGenerator world_generator;

    float yaw, pitch;

    GameState();
    ~GameState();

    void main_loop(const input::Input& input, float delta);
};
