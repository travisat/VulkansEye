#version 450

layout(binding = 0) uniform UniformVertex
{
    mat4 modelMVP;
    mat4 lightMVP;
} vertexBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 lightWorldPos;

const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0, //
                          0.0, 0.5, 0.0, 0.0, //
                          0.0, 0.0, 1.0, 0.0, //
                          0.5, 0.5, 0.0, 1.0);

void main()
{
    outUV = inUV;
    outNormal = inNormal;
    outPosition = inPosition;
    lightWorldPos = biasMat * vertexBuffer.lightMVP * vec4(inPosition, 1.0);
    gl_Position = vertexBuffer.modelMVP * vec4(inPosition, 1.0);
}