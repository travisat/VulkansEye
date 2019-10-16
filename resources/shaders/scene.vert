#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 model;
    vec3 eyeposition;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;


void main() {
    outUV = inUV;
    outNormal = inNormal;
    gl_Position = vec4(inPosition, 1.0);
    //gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}