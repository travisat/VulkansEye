#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	float tessStrength;
} ubo; 

layout (binding = 2) uniform sampler2D displacementMap; 

layout(triangles, equal_spacing, cw) in;

layout (location = 0) in vec2 inUV[];
layout (location = 1) in vec3 inNormal[];
 
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;

vec4 lerp3Dvec4(vec4 v0, vec4 v1, vec4 v2)
{
	 return vec4(gl_TessCoord.x) * v0 + vec4(gl_TessCoord.y) * v1 + vec4(gl_TessCoord.z) * v2;
}

vec3 lerp3Dvec3(vec3 v0, vec3 v1, vec3 v2)
{
	 return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

vec2 lerp3Dvec2(vec2 v0, vec2 v1, vec2 v2)
{
	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

void main()
{
	gl_Position = lerp3Dvec4(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position); 
	outUV = lerp3Dvec2(inUV[0], inUV[1], inUV[2]);
	outNormal = lerp3Dvec3(inNormal[0], inNormal[1], inNormal[2]);
				
	gl_Position.xyz += normalize(outNormal) * (max(textureLod(displacementMap, outUV.st, 0.0).r, 0.0) * ubo.tessStrength);
				
	outPosition = (gl_Position).xyz;
		
	gl_Position = ubo.projection * ubo.view * ubo.model * gl_Position;
}