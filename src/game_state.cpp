#include "game_state.hpp"

#include <fmt/core.h>

#include <registry.hpp>

using Registry = registry::Registry;
using namespace input;

GameState::GameState() {
    yaw = 0.0f;
    pitch = 0.0f;

    Registry::create();

    Registry& registry = Registry::get_mut();
    registry.register_json_file("/home/czebosak/Development/cpp/graphics/vulkan/example.json");

    registry.lock();

    chunk.mesh_state = voxel::Dirty {};
    chunk.data[0][0].fill(voxel::Block { registry.get_block("skibidi:air")->id });
    chunk.data[0].fill(chunk.data[0][0]);
    chunk.data.fill(chunk.data[0]);

    chunk.data[0][10][5].id = registry.get_block("skibidi:dirt")->id;
}

GameState::~GameState() {
    Registry::destroy();
}

void GameState::main_loop(Input& input) {
    glm::vec2 mouse_delta = input.get_mouse_delta();
    if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f) {
        yaw -= mouse_delta.x * 0.001f;
        pitch += mouse_delta.y * 0.001f;
        pitch = glm::clamp(pitch, glm::radians(-80.0f), glm::radians(80.0f));
        mouse_delta = glm::vec2(0.0f);
    }

    glm::quat q_yaw   = glm::angleAxis(yaw, glm::vec3(0,1,0));
    glm::quat q_pitch = glm::angleAxis(pitch, glm::vec3(1,0,0));
    camera.rotation = glm::normalize(q_yaw * q_pitch);

    glm::vec3 input_dir(0.0f);

    if (input.is_key_pressed(Key::KEY_W)) {
        input_dir.x += 1.0f;
    }
    if (input.is_key_pressed(Key::KEY_S)) {
        input_dir.x -= 1.0f;
    }
    if (input.is_key_pressed(Key::KEY_A)) {
        input_dir.z -= 1.0f;
    }
    if (input.is_key_pressed(Key::KEY_D)) {
        input_dir.z += 1.0f;
    }

    glm::vec3 dir = glm::length(input_dir) == 0.0f ? input_dir : glm::normalize(input_dir);

    glm::vec3 forward = camera.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right   = camera.rotation * glm::vec3(1.0f, 0.0f,  0.0f);

    glm::vec3 movement = (dir.x * forward + dir.z * right); //* 20.0f;
    camera.position += movement;
}
