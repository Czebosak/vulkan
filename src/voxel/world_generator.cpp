#include "world_generator.hpp"

#include <ranges>

using namespace voxel;

constexpr const char* NOISE_NODE_TREE = "DQAEAAAAAAAAQCkAAAAAAD8AAAAAAA==";

WorldGenerator::WorldGenerator() : seed(42) {}

Chunk WorldGenerator::generate_chunk(glm::ivec3 chunk_position, const registry::Registry& registry) const {
    //FastNoise::SmartNode<> noise = FastNoise::NewFromEncodedNodeTree( "DQAFAAAAAAAAQAgAAAAAAD8AAAAAAA==");
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

    fnFractal->SetSource( fnSimplex );
    fnFractal->SetOctaveCount( 4 );

    Chunk chunk = {
        .mesh = { .state = MeshState::Dirty }
    };

    //float heights[CHUNK_SIZE][CHUNK_SIZE];
    //fnFractal->GenUniformGrid2D(&heights[0][0], static_cast<float>(chunk_position.x * CHUNK_SIZE), static_cast<float>(chunk_position.z * CHUNK_SIZE), CHUNK_SIZE, CHUNK_SIZE, 1.0f, 1.0f, seed);

    auto iota = std::views::iota(size_t(0), CHUNK_SIZE);

    BlockID air = registry.get_block("skibidi:air")->id;
    BlockID dirt = registry.get_block("skibidi:dirt")->id;
    BlockID grass = registry.get_block("skibidi:grass")->id;

    for (auto [x, y, z] : std::views::cartesian_product(iota, iota, iota)) {
        //chunk.data[x][y][z] = Block { dirt };
        if (y + (chunk_position.y * CHUNK_SIZE) == 3) {
            chunk.data[x][y][z] = Block { grass };
        } else if ((y + (chunk_position.y * CHUNK_SIZE) < 3) && y + (chunk_position.y * CHUNK_SIZE) >= 0) {
            chunk.data[x][y][z] = Block { dirt };
        } else {
            chunk.data[x][y][z] = Block { air };
        }
        /* if (y + chunk_position.y * CHUNK_SIZE < heights[x][z] * 4.0f) {
            chunk.data[x][y][z] = Block { air };
        } else {
            chunk.data[x][y][z] = Block { dirt };
        } */
    }

    return chunk;
}
