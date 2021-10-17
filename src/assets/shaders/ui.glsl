#type vertex
#version 460
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in int in_texture_index;
layout(location = 0) out vec2 uv;
layout(location = 1) flat out int texture_index;
void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);
    uv = in_uv;
    texture_index = in_texture_index;
}
#type fragment
#version 460
layout(location = 0) in vec2 uv;
layout(location = 1) flat in int texture_index;
layout(location = 0) out vec4 fragment_color;
layout(binding = 0) uniform sampler2D textures[30];
void main() {
    fragment_color = texture(textures[texture_index], uv);
}