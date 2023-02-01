#version 450 core

vec3 positions[6] = {
    // first triangle
    vec3(1.0f, 1.0f, 0.0f),   // top right
    vec3(1.0f, -1.0f, 0.0f),  // bottom right
    vec3(-1.0f, 1.0f, 0.0f),  // top left
                              // second triangle
    vec3(1.0f, -1.0f, 0.0f),  // bottom right
    vec3(-1.0f, -1.0f, 0.0f), // bottom left
    vec3(-1.0f, 1.0f, 0.0f)   // top left
};

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
}