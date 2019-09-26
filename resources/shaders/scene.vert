#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 model;
    mat4 view;
    mat4 campos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    vec3 location = vec3(ubo.model * vec4(inPosition, 1.0));
    gl_Position = ubo.proj * ubo.view *  vec4(location, 1.0);
    outUV = inUV;
   
}