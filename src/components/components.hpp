#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform {
    glm::dvec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Velocity {
    glm::vec3 value;
};

struct Entity {};
