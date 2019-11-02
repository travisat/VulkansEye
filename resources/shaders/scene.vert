#version 450

layout(binding = 0) uniform UBO
{
    mat4 mvp;
    mat4 sunMVP;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 sunPosition;

const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0, //
                          0.0, 0.5, 0.0, 0.0, //
                          0.0, 0.0, 1.0, 0.0, //
                          0.5, 0.5, 0.0, 1.0);

void main()
{
    outUV = inUV;
    outNormal = inNormal;
    outPosition = inPosition;
    sunPosition = biasMat * ubo.sunMVP * vec4(inPosition, 1.0);
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);
}