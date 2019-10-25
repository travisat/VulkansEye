#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(binding = 0) uniform UniformBuffer
{
    mat4 projection;
    mat4 view;
    mat4 model;
}
ubo;

layout(location = 0) out vec4 outPosition;

void main()
{
    outPosition = ubo.model * vec4(inPosition, 1.0);
    gl_Position = outPosition;
}