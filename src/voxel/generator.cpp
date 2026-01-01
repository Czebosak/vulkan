#include "generator.hpp"

WorldGenerator::WorldGenerator() {
    noise = FastNoise::NewFromEncodedNodeTree("DQAEAAAAAAAAQCkAAAAAAD8AAAAAAA==");
}
