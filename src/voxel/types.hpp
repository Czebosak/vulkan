#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

using BlockID = uint32_t;

struct Block {
    BlockID id;
};

constexpr size_t CHUNK_SIZE = 16;

class Chunk {
private:
    std::array<std::array<std::array<Block, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> data;
    bool dirty;
public:
    void generate_mesh();
};
