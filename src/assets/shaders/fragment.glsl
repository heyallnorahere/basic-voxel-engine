#version 330 core
in vec2 uv;
uniform sampler2D block_texture;
out vec4 fragment_color;
void main() {
    fragment_color = vec4(1.0); // temporary
}