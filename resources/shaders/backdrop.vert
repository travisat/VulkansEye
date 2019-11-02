#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 mvp;
}
ubo;

// camera is always at center of skybox
layout(location = 0) out vec3 eyeDirection; 

void main()
{
    vec2 UV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 position = vec4(UV * 2.0f + -1.0f, 0.0f, 1.0f);

    eyeDirection = vec3(ubo.mvp * position);

    gl_Position = position;
}