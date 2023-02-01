#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 color;
layout(location = 4) in mat4 instance_model;

layout(binding = 0) uniform ubo
{
    mat4 view;
    mat4 proj;
};

void main()
{
}