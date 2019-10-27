#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 lightPos;

layout(location = 0) out float outColor;

void main()
{
    outColor = length(inPosition.xyz - lightPos.xyz);
}