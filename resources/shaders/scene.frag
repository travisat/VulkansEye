#version 450
#extension GL_ARB_separate_shader_objects : enable

//using source from
//https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/pbrtexture/pbrtexture.frag
//https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr_khr.frag

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 model;
    vec3 campos;
	vec2 texScale;
} ubo;

layout(binding = 1) uniform UniformLightObject
{
    vec3 position[2];
    vec3 color[2];
	float lumens[2];
	float numlights;
    float exposure;
    float gamma;
} ulo;

layout(binding = 2) uniform sampler2D diffuseMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D metallicMap;
layout(binding = 5) uniform sampler2D roughnessMap;
layout(binding = 6) uniform sampler2D aoMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

const float M_PI = 3.141592653589793;

struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float LdotH;                  // cos angle between light direction and half vector
	float VdotH;                  // cos angle between view direction and half vector
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};

#define MANUAL_SRGB 1
vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	#ifdef MANUAL_SRGB
	#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
	#else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	#endif //SRGB_FAST_APPROXIMATION
	return vec4(linOut,srgbIn.w);;
	#else //MANUAL_SRGB
	return srgbIn;
	#endif //MANUAL_SRGB
}

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal(vec2 inUV)
{
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = SRGBtoLINEAR(texture(normalMap, inUV)).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inPosition);
	vec3 q2 = dFdy(inPosition);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 lambertianDiffuse(vec3 diffuse)
{
	return diffuse / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(vec3 reflectance0, vec3 reflectance90, float VdotH)
{
	return reflectance0 + (reflectance90 - reflectance0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float geometricOcclusion(float NdotL, float NdotV, float roughness)
{
	float alpha = roughness * roughness;
	float k = ((alpha + 1.0) * (alpha + 1.0)) / 8;
	float ggxL = GeometrySchlickGGX(NdotL, k);
	float ggxV= GeometrySchlickGGX(NdotV, k);
	return ggxL * ggxV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(float NdotH, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	//float f = (NdotH * alphaSq - NdotH) * NdotH + 1.0;
	float nom = alphaSq;
	float denom = (NdotH * (alphaSq - 1.0) + 1.0);
	denom = M_PI * denom * denom;
	return nom / denom;
}

void main() {
	vec3 localCamPos = ubo.campos;

    float roughness = texture(roughnessMap, inUV).r;
    float metallic = texture(metallicMap, inUV).r;
	float ambientOcclusion = texture(aoMap, inUV).r;
    vec3 baseColor = SRGBtoLINEAR(texture(diffuseMap, inUV)).rgb;

	vec3 N = getNormal(inUV);
	vec3 V = normalize(localCamPos - inPosition);    // Vector from surface point to camera

	vec3 f0 = vec3(0.04);
	vec3 specularColor = mix(f0, baseColor, metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 directLighting = vec3(0.0);
	//for (int i = 1; i < 2; ++i){
		//vec3 lightPos = ulo.position[1];
		vec3 lightPos = ubo.campos;
		vec3 lightcolor = ulo.color[0];
		float lumens = ulo.lumens[0];
		
		vec3 L = normalize(lightPos - inPosition);     // Vector from surface point to light
		vec3 H = normalize(L+V);                        // Half vector between l and v
   
	    float NdotL = max(0.0, dot(N, L)); //cos angle between normal and light direction
		float NdotV = max(0.001, abs(dot(N, V))); //cos angle between nornal and view direction
		float NdotH = max(0.001, dot(N, H)); //cos angle between normal and half vector
		float LdotH = max(0.001, dot(L, H)); //cos angle between light direction and half vector
		float VdotH = max(0.001, dot(V, H)); //cos angle between view angle and half vector

	    vec3 F = specularReflection(specularEnvironmentR0, specularEnvironmentR90, VdotH); 
	    float D = microfacetDistribution(NdotH, roughness);
	    float G = geometricOcclusion(NdotL, NdotV, roughness);

		vec3 diffuseColor = baseColor * (vec3(1.0) - f0);
		diffuseColor *= 1.0 - metallic;

	    vec3 diffuseContrib = (1.0 - F) * lambertianDiffuse(diffuseColor);
	    vec3 specContrib = F * G * D / max(4.0 * NdotL * NdotV, 0.001);

		float intensity = lumens * lumens;
		lightcolor = lightcolor * intensity;

		directLighting += NdotL * lightcolor * (diffuseContrib + specContrib);
//}

	const float u_OcclusionStrength = 1.0f;
	vec3 ambientLight = vec3(0.04) * baseColor;
	ambientLight = mix(ambientLight, ambientLight * ambientOcclusion, u_OcclusionStrength);

    vec3 color = directLighting + ambientLight;

    outColor = vec4(color, 1.0);
} 