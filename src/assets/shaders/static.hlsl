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
#type fragment
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
struct texture_dimensions_struct {
    int2 atlas_position, texture_dimensions;
};
struct texture_atlas {
    Texture2D texture_;
    int2 texture_size, atlas_size;
    texture_dimensions_struct texture_dimensions_array[64]; // im gonna increase the size of this array once i add more blocks
};
light lights[30];
int light_count;
texture_atlas atlas;
float3 camera_position;
float4 main(PIXEL_IN input) : COLOR {
    return 0.f;
}