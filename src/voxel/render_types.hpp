#pragma once
#include <glm/glm.hpp>

namespace voxel {
    struct Vertex {
        glm::vec3 position;
        float _pad;
    };

    struct VoxelPushConstants {
        glm::mat4 mvp;
        VkDeviceAddress face_buffer;
    };
    
    using PackedFace = uint32_t;
}
