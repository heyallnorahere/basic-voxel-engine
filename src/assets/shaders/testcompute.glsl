#type compute
#version 460
layout(std430, binding = 0) buffer cpu_data {
    int input_integer, output_integer;
} data;
void main() {
    data.output_integer = data.input_integer * 7;
}