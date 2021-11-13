#type vertex
#version 460
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_voxel_position;
layout(location = 4) in int in_block_id;
layout(set = 0, binding = 0) uniform vertex_uniform_buffer_t {
    mat4 projection;
    mat4 view;
} vertex_uniform_buffer;
layout(location = 0) out vec2 uv;
layout(location = 1) flat out int block_id;
layout(location = 2) out vec3 fragment_position;
layout(location = 3) out vec3 normal;
void main() {
    gl_Position = vertex_uniform_buffer.projection * vertex_uniform_buffer.view * vec4(in_voxel_position + in_position, 1.0);
    uv = in_uv;
    block_id = in_block_id;
    normal = normalize(in_normal);
    fragment_position = in_voxel_position + in_position;
}
#type fragment
#version 460
layout(location = 0) in vec2 uv;
layout(location = 1) flat in int block_id;
layout(location = 2) in vec3 fragment_position;
layout(location = 3) in vec3 normal;
struct light_t {
    int type;
    vec4 position, color;
    float ambient_strength, specular_strength;
    // spotlight fields
    vec4 direction;
    float cutoff;
    // point light fields
    float constant, linear_, quadratic;
};
struct texture_dimensions_t {
    ivec2 grid_position, texture_dimensions;
};
struct texture_atlas_t {
    int image;
    ivec2 texture_size, grid_size;
    texture_dimensions_t texture_dimensions[64]; // im gonna increase the size of this array once i add more blocks
};
layout(set = 1, binding = 0) uniform fragment_uniform_buffer_t {
    light_t lights[30];
    int light_count;
    texture_atlas_t texture_atlas;
    vec4 camera_position;
} fragment_uniform_buffer;
layout(set = 1, binding = 1) uniform sampler2D textures[30];
layout(location = 0) out vec4 fragment_color;
vec4 get_texture() {
    texture_dimensions_t dimensions = fragment_uniform_buffer.texture_atlas.texture_dimensions[block_id];
    vec2 uv_offset = vec2(dimensions.grid_position) / vec2(fragment_uniform_buffer.texture_atlas.grid_size);
    vec2 uv_scale = vec2(dimensions.texture_dimensions) / vec2(fragment_uniform_buffer.texture_atlas.texture_size);
    vec2 uv_coordinates = (uv * uv_scale) + uv_offset;
    int texture_index = fragment_uniform_buffer.texture_atlas.image;
    return texture(textures[texture_index], uv_coordinates);
}
vec3 calculate_ambient(light_t l) {
    return l.ambient_strength * l.color.xyz;
}
vec3 calculate_diffuse(light_t l) {
    vec3 light_direction = normalize(l.position.xyz - fragment_position);
    float diff = max(dot(normal, light_direction), 0.0);
    return diff * l.color.xyz;
}
vec3 calculate_specular(light_t l) {
    vec3 light_direction = normalize(l.position.xyz - fragment_position);
    vec3 view_direction = normalize(fragment_uniform_buffer.camera_position.xyz - fragment_position);    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    return l.specular_strength * spec * l.color.xyz;
}
vec3 calculate_spotlight(light_t l, vec3 color, vec3 ambient_color) {
    vec3 light_direction = normalize(l.position.xyz - fragment_position);
    float theta = dot(light_direction, normalize(-l.direction.xyz));
    if (theta > l.cutoff) {
        return color;
    } else {
        return ambient_color;
    }
}
vec3 calculate_point_light(light_t l, vec3 color) {
    float distance_ = length(l.position.xyz - fragment_position);
    float distance_squared = distance_ * distance_;
    float attenuation = 1.0 / (l.constant + l.linear_ * distance_ + l.quadratic * distance_squared);
    return color * attenuation;
}
vec3 calculate_light(int index, vec3 _fragment_color) {
    light_t l = fragment_uniform_buffer.lights[index];
    vec3 ambient = calculate_ambient(l);
    vec3 diffuse = calculate_diffuse(l);
    vec3 specular = calculate_specular(l);
    vec3 color = (ambient + diffuse + specular) * _fragment_color;
    switch (l.type) {
    case 1: // spotlight
        return calculate_spotlight(l, color, ambient * _fragment_color);
    case 2: // point light
        return calculate_point_light(l, color);
    default:
        return vec3(1.0);
    }
}
void main() {
    vec4 color = get_texture();
    vec3 output_color = vec3(0.0);
    for (int i = 0; i < fragment_uniform_buffer.light_count; i++) {
        output_color += calculate_light(i, color.rgb);
    }
    fragment_color = vec4(output_color, color.a);
}