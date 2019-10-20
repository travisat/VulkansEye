#version 450

// http://onrendering.blogspot.com/2011/12/tessellation-on-gpu-curved-pn-triangles.html

// Phong tess patch data
struct PhongPatch
{
    float termIJ;
    float termJK;
    float termIK;
};

layout(binding = 1) uniform UBO
{
    mat4 projection;
    mat4 view;
    mat4 model;
    float tessStrength;
    float tessAlpha;
}
ubo;

layout(binding = 2) uniform sampler2D displacementMap;

layout(triangles, equal_spacing, cw) in;

layout(location = 0) in vec2 inUV[];
layout(location = 1) in vec3 inNormal[];
layout(location = 2) in PhongPatch inPhongPatch[];

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;

#define Pi gl_in[0].gl_Position.xyz
#define Pj gl_in[1].gl_Position.xyz
#define Pk gl_in[2].gl_Position.xyz
#define tc1 gl_TessCoord

void main()
{
    // precompute squared tesscoords
    vec3 tc2 = tc1 * tc1;

    // compute texcoord and normal
    outUV = gl_TessCoord[0] * inUV[0] + gl_TessCoord[1] * inUV[1] + gl_TessCoord[2] * inUV[2];
    outNormal = gl_TessCoord[0] * inNormal[0] + gl_TessCoord[1] * inNormal[1] + gl_TessCoord[2] * inNormal[2];

    // interpolated position
    vec3 barPos = gl_TessCoord[0] * Pi + gl_TessCoord[1] * Pj + gl_TessCoord[2] * Pk;

    // build terms
    vec3 termIJ = vec3(inPhongPatch[0].termIJ, inPhongPatch[1].termIJ, inPhongPatch[2].termIJ);
    vec3 termJK = vec3(inPhongPatch[0].termJK, inPhongPatch[1].termJK, inPhongPatch[2].termJK);
    vec3 termIK = vec3(inPhongPatch[0].termIK, inPhongPatch[1].termIK, inPhongPatch[2].termIK);

    // phong tesselated pos
    vec3 phongPos = tc2[0] * Pi + tc2[1] * Pj + tc2[2] * Pk + tc1[0] * tc1[1] * termIJ + tc1[1] * tc1[2] * termJK +
                    tc1[2] * tc1[0] * termIK;

    // final position
    outPosition = (1.0 - ubo.tessAlpha) * barPos + ubo.tessAlpha * phongPos;
    outPosition += normalize(outNormal) * (max(textureLod(displacementMap, outUV.st, 0.0).r, 0.0) * ubo.tessStrength);
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(outPosition, 1.0);
}