#version 330 core
in vec2 uv;
flat in int block_id;
in vec3 fragment_position;
in vec3 normal;
struct light {
    int type;
    vec3 position, color;
    float ambient_strength, specular_strength;
    // spotlight fields
    vec3 direction;
    float cutoff;
    // point light fields
    float constant, linear, quadratic;
};
struct texture_dimensions_struct {
    ivec2 atlas_position, texture_dimensions;
};
struct texture_atlas {
    sampler2D texture;
    ivec2 texture_size, atlas_size;
    texture_dimensions_struct texture_dimensions_array[64]; // im gonna increase the size of this array once i add more blocks
};
uniform light lights[30];
uniform int light_count;
uniform texture_atlas atlas;
uniform vec3 camera_position;
out vec4 fragment_color;
vec4 get_texture() {
    texture_dimensions_struct dimensions = atlas.texture_dimensions_array[block_id];
    vec2 uv_offset = vec2(dimensions.atlas_position) / vec2(atlas.atlas_size);
    vec2 uv_scale = vec2(dimensions.texture_dimensions) / vec2(atlas.texture_size);
    vec2 uv_coordinates = (uv * uv_scale) + uv_offset;
    return texture(atlas.texture, uv_coordinates);
}
vec3 calculate_ambient(light l) {
    return l.ambient_strength * l.color;
}
vec3 calculate_diffuse(light l) {
    vec3 light_direction = normalize(l.position - fragment_position);
    float diff = max(dot(normal, light_direction), 0.0);
    return diff * l.color;
}
vec3 calculate_specular(light l) {
    vec3 light_direction = normalize(l.position - fragment_position);
    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
    return l.specular_strength * spec * l.color;
}
vec3 calculate_spotlight(light l, vec3 color, vec3 ambient_color) {
    vec3 light_direction = normalize(l.position - fragment_position);
    float theta = dot(light_direction, normalize(-l.direction));
    if (theta > l.cutoff) {
        return color;
    } else {
        return ambient_color;
    }
}
vec3 calculate_point_light(light l, vec3 color) {
    float distance_ = length(l.position - fragment_position);
    float distance_squared = distance_ * distance_;
    float attenuation = 1.0 / (l.constant + l.linear * distance_ + l.quadratic * distance_squared);
    return color * attenuation;
}
vec3 calculate_light(int index, vec3 _fragment_color) {
    light l = lights[index];
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
    vec3 output = vec3(0.0);
    for (int i = 0; i < light_count; i++) {
        output += calculate_light(i, vec3(color));
    }
    fragment_color = vec4(output, color.a);
}