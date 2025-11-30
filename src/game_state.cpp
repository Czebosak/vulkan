#include "game_state.hpp"

#include <fmt/core.h>

#include <registry.hpp>

using Registry = registry::Registry;

GameState::GameState() {
    Registry::create();

    Registry& registry = Registry::get_mut();
    registry.register_json_file("/home/czebosak/Development/cpp/graphics/vulkan/example.json");

    registry.lock();

    const registry::BlockDefinition* def = registry.get_block("skibidi:air");
    fmt::println("{}, {}", def->id, def->name);

    def = registry.get_block("skibidi:grass");
    fmt::println("{}, {}", def->id, def->name);

    def = registry.get_block("skibidi:dirt");
    fmt::println("{}, {}", def->id, def->name);
}

GameState::~GameState() {
    Registry::destroy();
}
