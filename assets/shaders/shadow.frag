#version 450

layout(location = 0) in vec4 inPosition;

layout(location = 0) out vec2 color;

void main()
{
    float depth = length(inPosition);
    float moment1 = depth;
    float moment2 = depth * depth;
    float dx = dFdx(depth);
    float dy = dFdy(depth);
    moment2 += 0.25F * (dx * dx + dy * dy);
    color = vec2(moment1, moment2);
}