#version 450

layout(location = 0) in vec4 inPosition;

layout(location = 0) out float color;

void main()
{
   color = length(inPosition);
}