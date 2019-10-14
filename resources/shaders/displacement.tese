#version 450

layout (binding = 2) uniform UBO 
{
	mat4 projection;
	mat4 model;
	float tessAlpha;
	float tessStrength;
} ubo; 

layout (binding = 3) uniform sampler2D displacementMap; 

layout(triangles, equal_spacing, ccw) in;

layout (location = 0) in vec3 inPosition[];
layout (location = 1) in vec2 inUV[];
layout (location = 2) in vec3 inNormal[];

 
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;



void main()
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) + (gl_TessCoord.y * gl_in[1].gl_Position) + (gl_TessCoord.z * gl_in[2].gl_Position); 
	outUV = gl_TessCoord.x * inUV[0] + gl_TessCoord.y * inUV[1] + gl_TessCoord.z * inUV[2];
	outNormal = gl_TessCoord.x * inNormal[0] + gl_TessCoord.y * inNormal[1] + gl_TessCoord.z * inNormal[2]; 
				
	gl_Position.xyz += normalize(outNormal) * (max(textureLod(displacementMap, outUV.st, 0.0).a, 0.0) * ubo.tessStrength);
				
	outPosition = (gl_Position).xyz;
		
	gl_Position = ubo.projection * ubo.model * gl_Position;
}