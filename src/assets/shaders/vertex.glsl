#version 330 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_voxel_position;
layout(location = 4) in int in_block_id;
uniform mat4 projection;
uniform mat4 view;
out vec2 uv;
flat out int block_id;
out vec3 fragment_position;
out vec3 normal;
void main() {
    gl_Position = projection * view * vec4(in_voxel_position + in_position, 1.0);
    uv = in_uv;
    block_id = in_block_id;
    normal = normalize(in_normal);
    fragment_position = in_voxel_position + in_position;
}