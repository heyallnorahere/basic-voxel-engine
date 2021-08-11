#version 330 core
in vec2 uv;
flat in int block_id;
struct texture_dimensions_struct {
    ivec2 atlas_position, texture_dimensions;
};
struct texture_atlas {
    sampler2D texture;
    ivec2 texture_size, atlas_size;
    texture_dimensions_struct texture_dimensions_array[64]; // im gonna increase the size of this array once i add more blocks
};
uniform texture_atlas atlas;
out vec4 fragment_color;
vec4 get_texture() {
    texture_dimensions_struct dimensions = atlas.texture_dimensions_array[block_id];
    vec2 uv_offset = vec2(dimensions.atlas_position) / vec2(atlas.atlas_size);
    vec2 uv_scale = vec2(dimensions.texture_dimensions) / vec2(atlas.texture_size);
    vec2 uv_coordinates = (uv * uv_scale) + uv_offset;
    return texture(atlas.texture, uv_coordinates);
}
void main() {
    fragment_color = get_texture(); // temporary
}