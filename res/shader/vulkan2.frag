#version 450 core

layout(location = 0) out vec4 result;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput albedo;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput position;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput normal;

void main()
{
    result = subpassLoad(position);
}