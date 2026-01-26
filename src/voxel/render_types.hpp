#pragma once
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace voxel {
    struct Vertex {
        glm::vec3 position;
        float _pad;
    };
    
    using PackedFace = uint32_t;
}
