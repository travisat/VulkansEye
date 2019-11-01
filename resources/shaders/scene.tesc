#version 450

// http://onrendering.blogspot.com/2011/12/tessellation-on-gpu-curved-pn-triangles.html

layout(binding = 0) uniform UBO
{
    float tessLevel;
}
ubo;

layout(vertices = 3) out;

layout(location = 0) in vec2 inUV[];
layout(location = 1) in vec3 inNormal[];

layout(location = 0) out vec2 outUV[3];
layout(location = 1) out vec3 outNormal[3];
layout(location = 2) out vec3 outPatch[3];

#define Pi gl_in[0].gl_Position.xyz
#define Pj gl_in[1].gl_Position.xyz
#define Pk gl_in[2].gl_Position.xyz

// I have no idea what this does
float PIi(int i, vec3 q)
{
    vec3 q_minus_p = q - gl_in[i].gl_Position.xyz;
    return q[gl_InvocationID] - dot(q_minus_p, inNormal[i]) * inNormal[i][gl_InvocationID];
}

void main()
{
    // get data
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];

    // compute patch data
    outPatch[gl_InvocationID].x = PIi(0, Pj) + PIi(1, Pi);
    outPatch[gl_InvocationID].y = PIi(1, Pk) + PIi(2, Pj);
    outPatch[gl_InvocationID].z = PIi(2, Pi) + PIi(0, Pk);

    // tesselate
    gl_TessLevelOuter[gl_InvocationID] = ubo.tessLevel;
    gl_TessLevelInner[0] = ubo.tessLevel;
}