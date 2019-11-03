#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outPosition;

layout(binding = 0) uniform UniformShadow
{
    mat4 m;
    mat4 vp;
}
shadowBuffer;

void main()
{
    outPosition = shadowBuffer.m * vec4(inPosition, 1.0);
    gl_Position =  shadowBuffer.vp * outPosition;
}