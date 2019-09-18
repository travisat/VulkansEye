#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 texCoord;

layout (binding = 0) uniform UniformBufferObject 
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	outUVW = inPos;	
	gl_Position = ubo.proj * ubo.view * vec4(inPos, 1.0);
}