#pragma once
#include <FastNoise/FastNoise.h>

class WorldGenerator {
private:
    FastNoise::SmartNode<> noise;

    explicit WorldGenerator();
};
