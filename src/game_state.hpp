#pragma once

#include <backends/generic/camera.hpp>

#include <input.hpp>

#include <voxel/types.hpp>

class GameState {
public:
    Camera camera;

    voxel::Chunk chunk;

    float yaw, pitch;

    GameState();
    ~GameState();

    void main_loop(input::Input& input);
};
