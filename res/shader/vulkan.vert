#version 450 core

layout(binding = 0) uniform UniformBuffer
{
    mat4 model;
    mat4 view;
    mat4 proj;
}
ubo;

void main()
{
}