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
    vec4 lights[2];
    float exposure;
    float gamma;
} ulo;

layout(binding = 2) uniform sampler2D diffuseMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D aoMap;
layout(binding = 5) uniform sampler2D roughnessMap;

layout(location = 0) in vec2 inUV; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;



#define PI 3.1415926535897932384626433832795
#define ALBEDO pow(texture(diffuseMap, inUV).rgb, vec3(2.2))

// See http://www.thetenthplanet.de/archives/1180
vec3 perturbNormal()
{
	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inWorldPos);
	vec3 q2 = dFdy(inWorldPos);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * ALBEDO / PI + spec) * dotNL;
	}

	return color;
}

void main() {
    vec3 N = perturbNormal();
    vec3 V = normalize(ubo.campos - inWorldPos);
    vec3 R = reflect(-V, N);

    float metallic = 0.9f; //nonmetallic, need to get metallicmap
	float roughness = texture(roughnessMap, inUV).r;

    vec3 F0 = vec3(0.04); 

    vec3 Lo = vec3(0.0);
    for (int i ; i < ulo.lights[i].length(); i++)
    {
        vec3 L = normalize(ulo.lights[i].xyz - inWorldPos);
        Lo += specularContribution(L, V, N, F0, metallic, roughness);
    }

	//vec3 diffuse = irradiance * ALBEDO;	
    vec3 diffuse = ALBEDO;

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	//vec3 specular = reflection * (F * brdf.x + brdf.y);

	// Ambient part
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;	  
	vec3 ambient = (kD * diffuse) * texture(aoMap, inUV).rrr;
	
	vec3 color = ambient + Lo;

	// Tone mapping
	color = Uncharted2Tonemap(color * ulo.exposure);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	// Gamma correction
	color = pow(color, vec3(1.0f / ulo.gamma));

	outColor = vec4(color, 1.0);

}