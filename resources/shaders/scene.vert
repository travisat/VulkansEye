#version 450

layout(binding = 0) uniform UBO
{
    mat4 mvp;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;

void main()
{
    outUV = inUV;
    outNormal = inNormal;
    outPosition = inPosition;
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);
}