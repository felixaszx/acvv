#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 color;
layout(location = 4) in mat4 instance_matrix;

layout(location = 0) out vec4 albedo_in;
layout(location = 1) out vec4 position_in;
layout(location = 2) out vec4 normal_in;

layout(binding = 0) uniform UniformBuffer
{
    mat4 model;
    mat4 view;
    mat4 proj;
}
ubo;

void main()
{
    albedo_in = vec4(color, 1);
    position_in = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
    normal_in = vec4(normal, 1);

    gl_Position = position_in;
}