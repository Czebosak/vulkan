#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) flat in ivec3 inVoxelPos;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D displayTexture;
layout(set = 1, binding = 0) uniform usampler3D blockDataImages[];

layout(push_constant) uniform constants {
    layout(offset = 72) uint blockDataImageIndex;
} PushConstants;

void main() {
    uint block_data = texelFetch(
        blockDataImages[nonuniformEXT(PushConstants.blockDataImageIndex)],
        inVoxelPos,
        0
    ).r;
    uint id = (block_data >> 17) & 0x7FFF;
    uint r = id & 0x1F;
    uint g = (id >> 5) & 0x1F;
    uint b = (id >> 10) & 0x1F;

    outFragColor = texture(displayTexture, inUV);
    outFragColor = vec4(1.0 - r / 32.0, 1.0 - g / 32.0, 1.0 - b / 32.0, 1.0);
}
