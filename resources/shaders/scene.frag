#version 450
#extension GL_ARB_separate_shader_objects : enable

//[0] https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/pbrtexture/pbrtexture.frag
//[1] https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr_khr.frag
//[2] https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
//[3] https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
//[4] https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
//[5] https://www.shadertoy.com/view/lsSXW1
//[6] https://github.com/google/filament/blob/master/shaders/src/light_punctual.fs

const int numLights = 1;

layout(binding = 1) uniform UniformLight
{
    vec3 position;
	float lumens;
	float temperature;
} uLight[numLights];

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

//[0] and [2]
//convert from srgb color profile to linear color profile
//use on basecolor before using in equations
vec3 SRGBtoLINEAR(vec3 srgbIn)
{
	vec3 bLess = step(0.04045, srgbIn);
	vec3 linOut = mix( srgbIn / 12.92, pow((srgbIn + 0.055) / 1.055, vec3(2.4)), bLess );
	return linOut;
}

//[0]
// Find the normal for this fragment, pulling from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

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

//[0],[2]
// The following equation models the Fresnel reflectance term of the spec equation (aka F())
vec3 fresnelSchlick(vec3 f0, float f90, float u)
{
	return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

//[0],[2]
// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
float SmithGGXCorrelated(float NdotL, float NdotV, float roughness)
{
	//  Original  formulation  of  G_SmithGGX  Correlated
	//  lambda_v                = (-1 + sqrt(alphaG2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	//  lambda_l                = (-1 + sqrt(alphaG2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	//  G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l);
	//  V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0f * NdotL * NdotV);
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	//  Caution: the "NdotL  *" and "NdotV  *" are  explicitely  inversed , this is not a mistake.
	float  Lambda_GGXV = NdotL * sqrt((-NdotV * alphaSq + NdotV) * NdotV + alphaSq);
	float  Lambda_GGXL = NdotV * sqrt((-NdotL * alphaSq + NdotL) * NdotL + alphaSq);
	
	return  0.5f / (Lambda_GGXV + Lambda_GGXL);
}

//[0]
// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float ndfGGX(float NdotH, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	float f = (NdotH * alphaSq - NdotH) * NdotH + 1;
	return alphaSq / (f * f);
}

//[0],[2],[4] 
//Creates diffusiion brdf and specular brdf and combines them
vec3 BRDF(vec3 N, vec3 V, vec3 L, vec3 baseColor, float roughness, float metallic)
{	
	float NdotL = clamp(dot(N, L), 0.0, 1.0);      //cos angle between normal and light direction
	float NdotV = clamp(abs(dot(N, V)), 0.00001, 1.0); //cos angle between nornal and view direction
	vec3 H = normalize(L + V);
	float NdotH = clamp(dot(N, H), 0.0, 1.0);        //cos angle between normal and half vector
	float LdotH = clamp(dot(L, H), 0.0, 1.0);        //cos angle between light direction and half vector
	float VdotH = clamp(dot(V, H), 0.0, 1.0);
	
	//diffuse brdf
	// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
	float f90 = 0.5 + 2.0 * LdotH * LdotH * roughness * roughness;
	vec3 f0 = vec3 (1.0f, 1.0f, 1.0f);
	float FL = fresnelSchlick(f0, f90, NdotL).r;
	float FV = fresnelSchlick(f0, f90, NdotV).r;
	float Fd = FL * FV;

	// specular brdf
	f0 = mix(vec3(0.04), baseColor, metallic); //mix color based off metallic
	float reflectance = max(max(f0.r, f0.g), f0.b);
	f90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 F = fresnelSchlick(f0, f90, LdotH); 
    float G = SmithGGXCorrelated(NdotL, NdotV, roughness);
	float D = ndfGGX(NdotH, roughness); //microfacet distribution 
	vec3 specularContrib = (G * F * D);
	vec3 diffuseContrib =  (1.0 - F) * ( baseColor  * (1 - metallic));

	return (diffuseContrib + specularContrib) / M_PI;
}

//[5] converts light temperurate in kelvens to RGB
vec3 ColorTemperatureToRGB(float temperatureInKelvins)
{
	vec3 retColor;
    temperatureInKelvins = clamp(temperatureInKelvins, 1000.0, 40000.0) / 100.0;
    if (temperatureInKelvins <= 66.0)
    {
        retColor.r = 1.0;
        retColor.g = clamp(0.39008157876901960784 * log(temperatureInKelvins) - 0.63184144378862745098, 0.0, 1.0);
    }
    else
    {
    	float t = temperatureInKelvins - 60.0;
        retColor.r = clamp(1.29293618606274509804 * pow(t, -0.1332047592), 0.0, 1.0);
        retColor.g = clamp(1.12989086089529411765 * pow(t, -0.0755148492), 0.0, 1.0);
    }
    if (temperatureInKelvins >= 66.0)
        retColor.b = 1.0;
    else if(temperatureInKelvins <= 19.0)
        retColor.b = 0.0;
    else
        retColor.b = clamp(0.54320678911019607843 * log(temperatureInKelvins - 10.0) - 1.19625408914, 0.0, 1.0);
    return retColor;
}

//[6]
float getSquareFalloffAttenuation(float distanceSquare, float lumens) {
    float factor = distanceSquare * (1 / lumens);
    float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
    // We would normally divide by the square distance here
    // but we do it at the call site
    return smoothFactor * smoothFactor;
}

//[6]
float getDistanceAttenuation(vec3 posToLight, float lumens) {
    float distanceSquare = dot(posToLight, posToLight);
    float attenuation = getSquareFalloffAttenuation(distanceSquare, lumens);
    // Assume a punctual light occupies a volume of 1cm to avoid a division by 0
    return attenuation * 1.0 / max(distanceSquare, 1e-4);
}

void main() {
    float roughness = texture(roughnessMap, inUV).r;
    float metallic = texture(metallicMap, inUV).r;
	float ambientOcclusion = texture(aoMap, inUV).r;
    vec3 baseColor = SRGBtoLINEAR(texture(diffuseMap, inUV).rgb);
	vec3 N = getNormal();
	vec3 V = normalize(-inPosition);   // Vector from camera (origin) to surface


	vec3 lightPos = uLight[0].position;
	float lumens = uLight[0].lumens;
	float temperature = uLight[0].temperature;
	
	vec3 lightcolor = ColorTemperatureToRGB(temperature); 
	vec3 lightVector = lightPos - inPosition; //vector from light to survace
	float sqrDist = dot(lightVector, lightVector);
	float intensity = lumens  * getDistanceAttenuation(lightVector, lumens) / (4 * M_PI);
	lightcolor = lightcolor * intensity;

	vec3 L = normalize(lightVector);     // Vector from light to surface
	vec3 BDRFoutput = BRDF(N, V, L, baseColor, roughness, metallic);
	vec3 luminance = BDRFoutput * lightcolor;

	vec3 finalColor = luminance * ambientOcclusion;
    outColor = vec4(finalColor, 1.0);
} 