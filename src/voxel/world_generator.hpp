#pragma once

#include <FastNoise/FastNoise.h>

#include <voxel/types.hpp>
#include <registry.hpp>

#include <glm/glm.hpp>

class WorldGenerator {
private:
    //FastNoise::SmartNode<> noise;
    int seed;
public:
    WorldGenerator();

    voxel::Chunk generate_chunk(glm::ivec3 chunk_position, const registry::Registry& registry) const;
};
