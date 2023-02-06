#version 450 core

layout(location = 0) out vec4 light_result;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput albedo;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput position;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normal;

layout(push_constant) uniform constants
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 camera_pos;
    float strength;

    float constant;
    float linear;
    float quadratic;
}
light_data;

void main()
{
    light_result = subpassLoad(normal);
}