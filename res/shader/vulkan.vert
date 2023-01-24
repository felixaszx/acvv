#version 450 core

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

vec3 positions[12] = {vec3(-0.5f, -0.5f, 0.0f), //
                      vec3(0.5f, -0.5f, 0.0f),  //
                      vec3(0.5f, 0.5f, 0.0f),   //
                      vec3(0.5f, 0.5f, 0.0f),   //
                      vec3(-0.5f, 0.5f, 0.0f),  //
                      vec3(-0.5f, -0.5f, 0.0f),
                      //
                      vec3(-0.5f, -0.5f, -0.5f), //
                      vec3(0.5f, -0.5f, -0.5f),  //
                      vec3(0.5f, 0.5f, -0.5f),   //
                      vec3(0.5f, 0.5f, -0.5f),   //
                      vec3(-0.5f, 0.5f, -0.5f),  //
                      vec3(-0.5f, -0.5f, -0.5f)};

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(positions[gl_VertexIndex], 1.0);

    albedo_in = vec4(1, 0, 0, 1);
    position_in = ubo.proj * ubo.view * ubo.model * vec4(positions[gl_VertexIndex], 1.0);
    normal_in = vec4(0, 1, 0, 1);
}