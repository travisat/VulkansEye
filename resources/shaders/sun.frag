#version 450

layout(location = 0) out float depth;

layout(location = 0) in vec4 inPosition;

void main() 
{	
	depth = gl_FragCoord.z;
}