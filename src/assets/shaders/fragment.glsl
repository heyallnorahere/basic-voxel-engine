#version 330 core
in vec2 uv;
in vec2 texture_offset;
uniform sampler2D texture_atlas;
out vec4 fragment_color;
void main() {
    fragment_color = vec4(1.0); // temporary
}