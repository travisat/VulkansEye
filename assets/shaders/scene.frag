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

layout(binding = 1) uniform UniformLights
{
    vec4 position;
    float radianceMipLevels;
    float exposure;
    float gamma;
    float shadowSize;
}
lights;

layout(binding = 2) uniform sampler2D shadowMap;
layout(binding = 3) uniform sampler2D diffuseMap;
layout(binding = 4) uniform sampler2D normalMap;
layout(binding = 5) uniform sampler2D roughnessMap;
layout(binding = 6) uniform sampler2D metallicMap;
layout(binding = 7) uniform sampler2D aoMap;
layout(binding = 8) uniform samplerCube irradianceMap;
layout(binding = 9) uniform samplerCube radianceMap;
layout(binding = 10) uniform sampler2D brdfMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 lightWorldPos;
layout(location = 4) in vec4 camPos;

layout(location = 0) out vec4 outColor;

//[0]
// Find the normal for this fragment
vec3 getNormal(vec3 position, vec3 normal)
{
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.F - 1.F;

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

vec2 sampleOffsets[9] = vec2[](vec2(1.F, 1.F), vec2(1.F, 0.F), vec2(1.F, -1.F), //
                               vec2(0.F, 1.F), vec2(0.F, 0.F), vec2(0.F, -1.F), //
                               vec2(-1.0, 1.F), vec2(-1.F, 0.F), vec2(-1.F, -1.F));

float shadowCalc(vec3 lightVec, vec3 normal)
{
    float shadow = 0.F;
    // compute bias based off NdotL
    float bias = max(0.05F * (1.F - dot(normal, normalize(lightVec))), 0.005F);
    int samples = 9;                           // number of samples in sampleOffsets
    float texelSize = 1.F / lights.shadowSize; // size of texel that will be checked

    float currentDistanceToLight = length(lightVec);
    for (int i = 0; i < samples; ++i)
    {
        float closestDistanceToLight = texture(shadowMap, lightWorldPos.xy + sampleOffsets[i] * texelSize).r;
        if (currentDistanceToLight - bias > closestDistanceToLight)
        {
            shadow += 0.5F; // shadow instensity 0.0 = no shadow, 1.0 = full shadow
        }
    }
    shadow /= float(samples); // average all the samples to create shadow intensity
    return 1.F - shadow;      // sub from 1.0 here instead of later
}

vec3 iblBRDF(vec3 N, vec3 V, vec3 baseColor, float roughness, float metallic)
{
    float NdotV = clamp(abs(dot(N, V)), 0.001F, 1.F);
    vec3 f0 = vec3(0.04F);

    // compute diffuse
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuseColor = (baseColor - f0) * (1.F - metallic);
    vec3 diffuse = irradiance * diffuseColor;

    // compute specular
    vec3 R = reflect(-V, N);
    
    vec3 radiance = textureLod(radianceMap, R, roughness * (lights.radianceMipLevels - 1)).rgb;
    vec2 brdf = texture(brdfMap, vec2(NdotV, 1.F - roughness)).rg;
    vec3 specular = radiance * (mix(f0, vec3(irradiance), metallic) * brdf.x + brdf.y);

    return diffuse + specular;
}

void main()
{
    vec3 baseColor = texture(diffuseMap, inUV).rgb;
    float metallic = texture(metallicMap, inUV).r;
    float roughness = texture(roughnessMap, inUV).r;
    float ambientOcclusion = texture(aoMap, inUV).r;

    vec3 N = getNormal(inPosition, inNormal);      // Normal vector
    vec3 V = normalize(vec3(camPos) - inPosition); // Vector from camera to model

    float shadow = shadowCalc(vec3(lights.position) - inPosition, N);
    vec3 ambient = iblBRDF(N, V, baseColor, roughness, metallic);
    outColor = vec4(shadow * ambient * ambientOcclusion, 1.F);
}