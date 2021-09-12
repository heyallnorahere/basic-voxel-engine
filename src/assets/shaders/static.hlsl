#type vertex
float4x4 projection;
float4x4 view;
struct VERTEX_IN {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 voxel_position;
    int block_id;
};
struct VERTEX_OUT {
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    int block_id;
    float3 fragment_position;
    float3 normal : NORMAL;
};
VERTEX_OUT main(VERTEX_IN input) {
    VERTEX_OUT output;
    output.fragment_position = input.voxel_position + input.position;
    output.position = mul(projection, mul(view, float4(output.fragment_position, 1.0)));
    output.uv = input.uv;
    output.block_id = input.block_id;
    output.normal = normalize(input.normal);
    return output;
}
#type pixel
struct PIXEL_IN {
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    int block_id;
    float3 fragment_position;
    float3 normal : NORMAL;
};
struct light {
    int type;
    float ambient_strength, specular_strength;
    // spotlight fields
    float3 direction;
    float cutoff;
    // point light fields (apparently linear is a keyword in hlsl??????)
    float constant, linear_, quadratic;
};
struct texture_dimensions_t {
    int2 atlas_position, texture_dimensions;
};
uniform light lights[30];
uniform int light_count;
uniform sampler2D texture_atlas;
uniform int2 texture_atlas_size, grid_size;
uniform texture_dimensions_t texture_dimensions_array[64]; // im gonna increase the size of this array once i add more blocks
uniform float3 camera_position;
float4 get_fragment_color(int block_id, float2 input_uv_coords) {
    texture_dimensions_t dimensions = texture_dimensions_array[block_id];
    float2 uv_offset = float2(dimensions.atlas_position) / float2(grid_size);
    float2 uv_scale = float2(dimensions.texture_dimensions) / float2(texture_atlas_size);
    float2 uv = (input_uv_coords * uv_scale) + uv_offset;
    return tex2D(texture_atlas, uv);
}
float4 main(PIXEL_IN input) : COLOR {
    return get_fragment_color(input.block_id, input.uv);
}