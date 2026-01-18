#include "interface.h"

#include <flecs.h>

static flecs::world* world;

struct Position {};

void register_system(void (*fn)(void*), const char* name, const char* query) {
/*     flecs::query q = world->query_builder()
        .expr("Position, [in] Velocity")
        .build();
    
    flecs::world w;
    
    world->system("Name").;
    flecs::id sys = world->entity();
    ecs_system_desc_t sys_desc = { .};
    flecs::system(world, sys); */
}
