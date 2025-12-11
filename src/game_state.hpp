#pragma once

#include <backends/generic/camera.hpp>

#include <input.hpp>

class GameState {
public:
    Camera camera;

    float yaw, pitch;

    GameState();
    ~GameState();

    void main_loop(input::Input& input);
};
