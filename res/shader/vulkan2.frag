#version 450 core

layout(location = 0) out vec4 present_result;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput screen;

void main()
{
    vec3 hdr_color = subpassLoad(screen).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * 0.1);
    mapped = pow(mapped, vec3(1.0 / 2.2));

    present_result = vec4(mapped, 1.0);
}