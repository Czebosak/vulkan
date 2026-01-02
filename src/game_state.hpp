#pragma once

#include <backends/generic/camera.hpp>

#include <input.hpp>

#include <voxel/types.hpp>
#include <voxel/world_generator.hpp>

class GameState {
public:
    Camera camera;

    voxel::Chunk chunk;

    WorldGenerator world_generator;

    float yaw, pitch;

    GameState();
    ~GameState();

    void main_loop(const input::Input& input, float delta);
};
