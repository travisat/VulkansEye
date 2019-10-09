#version 450
#extension GL_ARB_separate_shader_objects : enable

//https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/pbrtexture/pbrtexture.frag

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 model;
    vec3 campos;
} ubo;

layout(binding = 1) uniform UniformLightObject
{
    vec3 position;
    vec3 color;
    float exposure;
    float gamma;
} ulo;

layout(binding = 2) uniform sampler2D diffuseMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D aoMap;
layout(binding = 5) uniform sampler2D roughnessMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {

	outColor = texture(diffuseMap, inUV);

} 