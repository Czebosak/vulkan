#version 450
#extension GL_EXT_buffer_reference : require

struct Face {
    vec3 vertices[4];
    vec3 normal;
};

const Face FACES[6] = Face[6](
    // +Y face (top)
    Face(
        vec3[4](
            /* vec3(0,1,0),
            vec3(1,1,0),
            vec3(0,1,1),
            vec3(1,1,1) */
            vec3(0,1,0),
            vec3(0,1,1),
            vec3(1,1,0),
            vec3(1,1,1)
        ),
        vec3(0,1,0)
    ),

    // -Y face (bottom)
    Face(
        vec3[4](
            vec3(0,0,0),
            vec3(1,0,0),
            vec3(0,0,1),
            vec3(1,0,1)
        ),
        vec3(0,-1,0)
    ),

    // -X face (left)
    Face(
        vec3[4](
            vec3(0,0,0),
            vec3(0,0,1),
            vec3(0,1,0),
            vec3(0,1,1)
        ),
        vec3(-1,0,0)
    ),

    // +X face (right)
    Face(
        vec3[4](
            /* vec3(1,0,0),
            vec3(1,0,1),
            vec3(1,1,0),
            vec3(1,1,1) */
            vec3(1,0,0),
            vec3(1,1,0),
            vec3(1,0,1),
            vec3(1,1,1)
        ),
        vec3(1,0,0)
    ),

    // +Z face (front)
    Face(
        vec3[4](
            vec3(0,0,1),
            vec3(1,0,1),
            vec3(0,1,1),
            vec3(1,1,1)
        ),
        vec3(0,0,1)
    ),

    // -Z face (back)
    Face(
        vec3[4](
            vec3(0,0,0),
            vec3(0,1,0),
            vec3(1,0,0),
            vec3(1,1,0)
        ),
        vec3(0,0,-1)
    )
);

const vec2 UV[4] = {
    vec2(0,0), // v0
    vec2(1,0), // v1
    vec2(0,1), // v2
    vec2(1,1)  // v3
};

struct UnpackedFaceData {
    uint face;
    ivec3 pos;
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) flat out ivec3 outVertexPos;

layout(buffer_reference, std430) readonly buffer FaceBuffer {
    uint faces[];
};

layout(push_constant) uniform constants {
    mat4 mvp;
    FaceBuffer face_buffer;
} PushConstants;

UnpackedFaceData unpack_face_data(uint packed) {
    return UnpackedFaceData(
        packed >> 15, // face
        ivec3( // Position
            packed & 0x1Fu,
            (packed >> 5) & 0x1Fu,
            (packed >> 10) & 0x1Fu
        )
    );
}

const uint VERTEX_COUNT = 4;

void main() {
    uint packed_face = PushConstants.face_buffer.faces[gl_InstanceIndex];

    UnpackedFaceData unpacked_face = unpack_face_data(packed_face);

    Face face = FACES[unpacked_face.face];

    outNormal = face.normal;
    outUV = UV[gl_VertexIndex];
    outVertexPos = unpacked_face.pos;

    gl_Position = PushConstants.mvp * vec4(face.vertices[gl_VertexIndex] + vec3(unpacked_face.pos), 1.0f);
}
