#version 450 core

layout(location = 0) in vec4 albedo_in;
layout(location = 1) in vec4 position_in;
layout(location = 2) in vec4 normal_in;

layout(location = 0) out vec4 albedo;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;

void main()
{
    albedo = albedo_in;
    position = position_in;
    normal = normal_in;
}