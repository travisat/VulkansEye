#version 450

struct PointLight
{
    vec3 position;
    vec3 color;
    float lumens;
};

layout(binding = 2) uniform UniformLight
{
    PointLight light[1];
}
uLight;

layout(location = 0) in vec4 inPosition;
layout(location = 0) out float outColor;

void main()
{
    vec3 position = inPosition.xyz;
    vec3 lightPos = uLight.light[0].position;

    outColor = distance(position, lightPos);
}