#version 450

layout(location = 0) in vec4 inPosition;

layout(location = 0) out float outColor;

void main()
{
    outColor = length(inPosition.xyz);
}