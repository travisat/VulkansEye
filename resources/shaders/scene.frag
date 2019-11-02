#version 450

//[0] https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/pbrtexture/pbrtexture.frag
//[1] https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr_khr.frag
//[2] https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
//[3] https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
//[4] https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
//[5] https://github.com/google/filament/blob/master/shaders/src/light_punctual.fs
//[6] https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
//[7] https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/shadowmapping/scene.frag
//[8] https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/pbribl/pbribl.frag

const int numLights = 1;

struct PointLight
{
    vec3 position;
    vec3 color;
    float lumens;
};

layout(binding = 1) uniform UniformLight
{
    vec3 sunAngle;
    float radianceMipLevels;
    float exposure;
    float gamma;
    float shadowSize;
    PointLight light[numLights];
}
uLight;

layout(binding = 2) uniform samplerCube shadowMap;
layout(binding = 3) uniform sampler2D diffuseMap;
layout(binding = 4) uniform sampler2D normalMap;
layout(binding = 5) uniform sampler2D metallicMap;
layout(binding = 6) uniform sampler2D roughnessMap;
layout(binding = 7) uniform sampler2D aoMap;
layout(binding = 8) uniform samplerCube irradianceMap;
layout(binding = 9) uniform samplerCube radianceMap;
layout(binding = 10) uniform sampler2D sunMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 sunPosition;

layout(location = 0) out vec4 outColor;

const float PI = 3.1415926;
const float InvPI = 0.3183109;

//[0] and [2]
// convert from srgb color profile to linear color profile
// for converting basecolor (diffuse)
vec3 convertSRGBtoLinear(vec3 srgbIn)
{
    vec3 bLess = step(0.04045, srgbIn);
    vec3 linOut = mix(srgbIn / 12.92, pow((srgbIn + 0.055) / 1.055, vec3(2.4)), bLess);
    return linOut;
}

//[0]
// Find the normal for this fragment, pulling from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal(vec3 position, vec3 normal)
{
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

    vec3 q1 = dFdx(position);
    vec3 q2 = dFdy(position);
    vec2 st1 = dFdx(inUV);
    vec2 st2 = dFdy(inUV);

    vec3 N = normalize(normal);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

//[0],[2]
// The following equation models the Fresnel reflectance term of the spec
// equation (aka F())
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
    //  lambda_v                = (-1 + sqrt(alphaG2 * (1 - NdotL2) / NdotL2 + 1))
    //  * 0.5f; lambda_l                = (-1 + sqrt(alphaG2 * (1 - NdotV2) /
    //  NdotV2 + 1)) * 0.5f; G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l);
    //  V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0f * NdotL * NdotV);
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    //  Caution: the "NdotL  *" and "NdotV  *" are  explicitely  inversed , this
    //  is not a mistake.
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaSq + NdotV) * NdotV + alphaSq);
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaSq + NdotL) * NdotL + alphaSq);

    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

//[0]
// The following equation(s) model the distribution of microfacet normals across
// the area being drawn (aka D()) Implementation from "Average Irregularity
// Representation of a Roughened Surface for Ray Reflection" by T. S.
// Trowbridge, and K. P. Reitz Follows the distribution function recommended in
// the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float ndfGGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float f = (NdotH * alphaSq - NdotH) * NdotH + 1;
    return alphaSq / (f * f);
}

vec3 diffuseBDRF(float NdotL, float NdotV, float roughness, float metallic, vec3 diffuse)
{
    // Diffuse brdf
    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // mix in diffuse retro-reflection based on roughness
    float f90 = 0.5 + 2.0 * NdotL * NdotL * roughness * roughness;
    vec3 f0 = vec3(1.0 - metallic);
    float FL = fresnelSchlick(f0, f90, NdotL).r;
    float FV = fresnelSchlick(f0, f90, NdotV).r;
    float Fd = FL * FV;

    return Fd * diffuse;
}

vec3 specularBDRF(float NdotL, float NdotV, float NdotH, float VdotH, float roughness, float metallic, vec3 diffuse,
                  vec3 light)
{
    // Specular brdf
    vec3 f0 = mix(vec3(0.04), diffuse, metallic); // mix color based off metallic
    float reflectance = max(max(f0.r, f0.g), f0.b);
    float f90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 F = fresnelSchlick(f0, f90, VdotH);
    float G = SmithGGXCorrelated(NdotL, NdotV, roughness);
    float D = ndfGGX(NdotH, roughness); // microfacet distribution
    return G * F * D * light; // div by pi in main BRDF
}

//[0],[2],[4]
vec3 BRDF(vec3 N, vec3 V, vec3 L, float roughness, float metallic, vec3 diffuse,
          vec3 light)
{
    // cos angle between normal and light direction
    float NdotL = clamp(dot(N, L), 0.00001, 1.0);
    // cos angle between nornal and view direction
    float NdotV = clamp(abs(dot(N, V)), 0.00001, 1.0);
    // half vector
    vec3 H = normalize(L + V);
    // cos angle between normal and half vector
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    // cos angle between light direction and half vector
    // float LdotH = clamp(dot(L, H), 0.0, 1.0);
    // cos angle between view direction and half vector
    float VdotH = clamp(dot(V, H), 0.0, 1.0);

    vec3 diffuseContrib = diffuseBDRF(NdotL, NdotV, roughness, metallic, diffuse);
    vec3 specularContrib = specularBDRF(NdotL, NdotV, NdotH, VdotH, roughness, metallic, diffuse, light);
    return (diffuseContrib + specularContrib) * InvPI;
}

//[5]
// Calculate attenuation
// lightvector is (inposition - lightposition)
// lumens are lumens
// angle is in steradians
// angle for a point light = 4 * PI
float getDistanceAttenuation(vec3 lightVector, float lumens, float angle)
{
    float distanceSquared = dot(lightVector, lightVector);
    // this puts light into inverted range
    float inverseLight = (distanceSquared) / lumens;
    // square smooth and subtract from 1 to get correct range
    float smoothLight = clamp(1.0 - inverseLight * inverseLight, 0.0, 1.0);
    smoothLight = smoothLight * smoothLight;
    // apply inverse square law to smoothed light to get attenuation
    return smoothLight / (angle * (distanceSquared + 1.0));
}

//[6] directions to sample in
vec3 shadowOffsetDirections[20] = vec3[](
    vec3(1, 1, 1), vec3(1, 1, 0), vec3(1, 1, -1), vec3(1, 0, 1), vec3(1, 0, -1), vec3(1, -1, 1), vec3(1, -1, 0),
    vec3(1, -1, -1), vec3(0, 1, 1), vec3(0, 1, -1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(-1, 1, 1), vec3(-1, 1, 0),
    vec3(-1, 1, -1), vec3(-1, 0, 1), vec3(-1, 0, -1), vec3(-1, -1, 1), vec3(-1, -1, 0), vec3(-1, -1, -1));

//[6]
float shadowCalc(vec3 lightVec)
{
    float shadow = 0.0; // initialize shadow to 0
    float bias = 0.15;  //
    int samples = 20;   // number of samples in shadowOffsetDirections
    float viewDistance = length(inPosition);
    float currentDepth = length(lightVec);
    // set radius to sample in based off distance from viewer
    // make shadows closer sharper
    // 512 is the maxlod from shadowmap so something 512 away and more will have a disc radius of 1.0
    float diskRadius = (1.0 + (viewDistance / 512.0)) / 50.0;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, lightVec + shadowOffsetDirections[i] * diskRadius).r;
        if (currentDepth + bias > closestDepth)
        {
            shadow += 1.0; // shadow instensity 0.0 = no shadow, 1.0 = full shadow, 6 looks nice
        }
    }
    shadow /= float(samples); // average all the samples to create shadow intensity
    return 1.0 - shadow;      // sub from 1.0 here instead of later
}

vec2 sunOffsetDirections[9] = vec2[](vec2(1, 1), vec2(1, 0), vec2(1, -1), vec2(0, 1), vec2(0, 0), vec2(0, -1),
                                     vec2(-1, 1), vec2(-1, 0), vec2(-1, -1));

float sunCalc(vec3 lightVec, vec3 normal)
{
    float shadow = 0.0;
    float bias = max(0.05 * (1.0 - dot(normal, normalize(lightVec))), 0.005);
    int samples = 9; // number of samples in sunOffsetDirections
    float currentDepth = length(lightVec);
    float texelSize = 1.0 / uLight.shadowSize;

    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(sunMap, sunPosition.xy + sunOffsetDirections[i] * texelSize).r;
        if (currentDepth - bias > closestDepth)
        {
            shadow += 0.6; // shadow instensity 0.0 = no shadow, 1.0 = full shadow, 6 looks nice
        }
    }
    shadow /= float(samples); // average all the samples to create shadow intensity
    return 1.0 - shadow;      // sub from 1.0 here instead of later
}

//[8]
vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(radianceMap, R, lodf).rgb;
	vec3 b = textureLod(radianceMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

void main()
{

    vec3 baseColor = convertSRGBtoLinear(texture(diffuseMap, inUV).rgb);
    float metallic = texture(metallicMap, inUV).r;
    float roughness = texture(roughnessMap, inUV).r;
    float ambientOcclusion = texture(aoMap, inUV).r;

    vec3 N = getNormal(inPosition, inNormal); // Normal vector
    vec3 V = normalize(inPosition);           // Vector from surface to camera(origin)

    float lod = (roughness * uLight.radianceMipLevels);

    vec3 sunvec = normalize(inPosition - uLight.sunAngle);
    vec3 reflection = reflect(-V, N);
    vec3 diffuseLight = texture(irradianceMap, N).rgb;
    vec3 specularLight = prefilteredReflection(reflection, roughness);

    float brightness = 10;
    vec3 ambient = BRDF(N, V, sunvec, roughness, metallic, baseColor * diffuseLight, specularLight);

    ambient = brightness * sunCalc(inPosition - uLight.sunAngle, N) * ambient;

    vec3 luminance = vec3(0.0);
    for (int i = 0; i < numLights; ++i)
    {
        vec3 lightPos = uLight.light[i].position;
        vec3 lightcolor = uLight.light[i].color;
        float lumens = uLight.light[i].lumens;

        vec3 lightVector = inPosition - lightPos;
        float intensity = lumens * getDistanceAttenuation(lightVector, lumens, 4 * PI);
        vec3 L = normalize(lightVector); // vector from surface to light
        luminance +=
            intensity * shadowCalc(lightVector) * BRDF(N, V, L, roughness, metallic, baseColor * lightcolor, lightcolor);
    }

    outColor = vec4((ambient)*ambientOcclusion, 1.0);
}