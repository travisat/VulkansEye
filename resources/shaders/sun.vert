#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(binding = 0) uniform UBO
{
    mat4 sunMVP;
}
ubo;

void main()
{
    gl_Position =  ubo.sunMVP * vec4(inPos, 1.0);
}