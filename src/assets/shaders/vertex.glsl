#version 330 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec2 in_texture_offset;
uniform mat4 projection;
uniform mat4 view;
out vec2 uv;
out vec2 texture_offset;
void main() {
    gl_Position = projection * view * vec4(in_position, 1.0);
    uv = in_uv;
    texture_offset = in_texture_offset;
}