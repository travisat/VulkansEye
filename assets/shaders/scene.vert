#version 450

layout(binding = 0) uniform UniformVertex
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightMVP;
    mat4 normalMatrix;
    vec4 camPos;
    float uvScale;
}
vertexBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 lightWorldPos;
layout(location = 4) out vec4 camPos;

const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0, //
                          0.0, 0.5, 0.0, 0.0, //
                          0.0, 0.0, 1.0, 0.0, //
                          0.5, 0.5, 0.0, 1.0);

void main()
{
    outUV = inUV * vertexBuffer.uvScale;
    outNormal = normalize(mat3(vertexBuffer.normalMatrix) * inNormal);
    camPos = vertexBuffer.camPos;
    
    outPosition =  vertexBuffer.model * vec4(inPosition, 1.0);
    lightWorldPos = biasMat * vertexBuffer.lightMVP * vec4(inPosition, 1.0);
    gl_Position = vertexBuffer.projection * vertexBuffer.view * outPosition;
}