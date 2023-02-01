#version 450 core

layout(location = 0) out vec4 light_result;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput albedo;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput position;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normal;

void main()
{
}