#type compute
#version 460
layout(std430, binding = 0) buffer uniform_data {
    int input_integer, output_integer;
} ubo;
void main() {
    ubo.output_integer = ubo.input_integer * 7;
}