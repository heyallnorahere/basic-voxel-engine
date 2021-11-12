#type compute
#version 460
layout(std140, set = 0, binding = 0) uniform playerData {
    vec3 position, direction;
} player;
struct vertex {
    vec3 position, normal;
};
struct modelData {
    int vertexCount, indexCount;
    vertex vertices[128];
    uint indices[128];
};
layout(std140, set = 0, binding = 1) uniform blockData {
    ivec3 position;
    bool hasModel;
    modelData model;
} block;
struct candidate {
    float distanceFromPlayer;
    ivec3 blockPosition;
};
layout(std430, set = 0, binding = 2) buffer outputData {
    int candidateCount;
    candidate candidates[64];
} outputBuffer;
vertex faceVertices[] = {
    { vec3(-0.5, -0.5, 0.5), vec3(0.0, 0.0, 1.0) },
    { vec3( 0.5, -0.5, 0.5), vec3(0.0, 0.0, 1.0) },
    { vec3( 0.5,  0.5, 0.5), vec3(0.0, 0.0, 1.0) },
    { vec3(-0.5,  0.5, 0.5), vec3(0.0, 0.0, 1.0) }
};
uint faceIndices[] = {
    0, 1, 3,
    1, 2, 3,  
};
vec3 normalBases[] = {
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
};
vec2 getEulerAngles(vec3 direction) {
    vec2 angle;
    angle.x = asin(direction.y);
    float factor = cos(angle.x);
    angle.y = atan(direction.z / factor, direction.x / factor);
}
vec3 getDirection(vec2 eulerAngles) {
    vec3 direction;
    direction.x = cos(eulerAngles.y) * cos(eulerAngles.x);
    direction.y = sin(eulerAngles.x);
    direction.z = sin(eulerAngles.y) * cos(eulerAngles.x);
    return normalize(direction);
}
// since i dont want to make a simple cube model and import it, im doing this shit
void assembleCubeFace(int faceIndex, out vertex vertices[3]) {
    int baseFaceIndex = faceIndex % 2;
    int normalIndex = (faceIndex - baseFaceIndex) / 2;
    vec3 faceNormal = normalBases[normalIndex % 3];
    if (normalIndex >= 3) {
        faceNormal *= -1.0;
    }
    vec2 normalAngle = getEulerAngles(faceNormal);
    for (int i = 0; i < 3; i++) {
        // this should theoretically work
        uint index = faceIndices[baseFaceIndex * 3 + i];
        vertex faceVertex = faceVertices[index];
        vec2 angleOffset = normalAngle - getEulerAngles(faceVertex.normal);
        vec2 vertexAngle = getEulerAngles(faceVertex.position);
        vec2 newVertexAngle = vertexAngle + angleOffset;
        vertices[i].position = getDirection(newVertexAngle);
        vertices[i].normal = faceNormal;
    }
}
void getModelVertices(out vertex vertices[3]) {
    int faceIndex = int(gl_GlobalInvocationID.x);
    if (block.hasModel) {
        modelData model = block.model;
        for (int i = 0; i < 3; i++) {
            uint index = model.indices[faceIndex * 3 + i];
            vertices[i] = model.vertices[index];
        }
    } else {
        assembleCubeFace(faceIndex, vertices);
    }
}
void main() {
    vertex vertices[3];
    getModelVertices(vertices);
}