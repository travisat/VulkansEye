#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outPosition;

layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 vp;
}
ubo;

void main()
{
    outPosition = ubo.model * vec4(inPosition, 1.0);
    gl_Position =  ubo.vp * outPosition;
}