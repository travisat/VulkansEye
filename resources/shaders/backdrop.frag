#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube cubeSampler;

layout(location = 0) in vec3 eyePosition;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 color = texture(cubeSampler, eyePosition).rgb;
    outColor = vec4(color, 1.0);
}