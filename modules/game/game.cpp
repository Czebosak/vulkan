#include <iostream>

#include "defs.hpp"

#include "../../external/flecs/include/flecs.h"

struct Player {
    float health;
};

extern "C" bool MODULE_EXPORT game_init(flecs::world world) {
    world.entity()
        .set<Player>({ .health = 5.0f });

    world.system<const Player>("Player Health Print")
    .each([](const Player& p) {
        std::cout << p.health << "\n";
    });

    world.system<Player>("Player Health Update")
    .each([](Player& p) {
        p.health -= 0.01f;
    });

    return true;
}
